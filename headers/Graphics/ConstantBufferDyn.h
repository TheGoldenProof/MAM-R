#pragma once
#include "TGLib/TGLib.h"
#include <cassert>
#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include <vector>

#define BASE_ELEMENT_TYPES \
	X(Float) \
	X(Float2) \
	X(Float3) \
	X(Float4) \
	X(Matrix) \
	X(Bool)
#define BET BASE_ELEMENT_TYPES

namespace CBD {
namespace dx = DirectX;

enum Type {
	#define X(t) t,
	BET
	#undef X
	Struct,
	Array,
	Empty
};

#pragma region Info Maps
template<Type type> struct Info {
	static constexpr bool valid = false;
};
template<> struct Info<Float> {
	using SysType = f32;
	static constexpr usize hlslSize = sizeof(SysType);
	static constexpr const char* code = "F1";
	static constexpr bool valid = true;
};
template<> struct Info<Float2> {
	using SysType = dx::XMFLOAT2;
	static constexpr usize hlslSize = sizeof(SysType);
	static constexpr const char* code = "F2";
	static constexpr bool valid = true;
};
template<> struct Info<Float3> {
	using SysType = dx::XMFLOAT3;
	static constexpr usize hlslSize = sizeof(SysType);
	static constexpr const char* code = "F3";
	static constexpr bool valid = true;
};
template<> struct Info<Float4> {
	using SysType = dx::XMFLOAT4;
	static constexpr usize hlslSize = sizeof(SysType);
	static constexpr const char* code = "F4";
	static constexpr bool valid = true;
};
template<> struct Info<Matrix> {
	using SysType = dx::XMMATRIX;
	static constexpr usize hlslSize = sizeof(SysType);
	static constexpr const char* code = "M4";
	static constexpr bool valid = true;
};
template<> struct Info<Bool> {
	using SysType = bool;
	static constexpr usize hlslSize = 4;
	static constexpr const char* code = "BL";
	static constexpr bool valid = true;
};

#define X(t) static_assert(Info<t>::valid, "Missing info for " #t);
BET
#undef X

template<typename T> struct ReverseInfo {
	static constexpr bool valid = false;
};
#define X(t) \
template<> struct ReverseInfo<typename Info<t>::SysType> { \
	static constexpr Type type = t; \
	static constexpr bool valid = true; \
};
BET
#undef X

#pragma endregion struct Info<Type>, struct ReverseInfo<typename>

class LayoutElement {
	friend class RawLayout;
	friend struct ExtraData;
private:
	struct ExtraDataBase {
		virtual ~ExtraDataBase() = default;
	};
public:
	std::string GetSignature() const dbgexcept;

	bool Exists() const noexcept;

	std::pair<usize, const LayoutElement*> CalculateIndexingOffset(usize offset, usize index) const dbgexcept;

	LayoutElement& operator[](const std::string& key) dbgexcept;
	const LayoutElement& operator[](const std::string& key) const dbgexcept;

	LayoutElement& T() dbgexcept;
	const LayoutElement& T() const dbgexcept;

	usize GetOffsetBegin() const dbgexcept;
	usize GetOffsetEnd() const dbgexcept;
	usize GetSizeInBytes() const dbgexcept;

	LayoutElement& Add(Type type, const std::string& name);
	LayoutElement& Set(Type type, usize size) dbgexcept;

	template<typename T>
	usize Resolve() const dbgexcept {
		switch (type) {
		#define X(t) case t: assert(typeid(Info<t>::SysType) == typeid(T)); return *offset;
		BET
		#undef X
		}
		assert("Tried to resolve invalid element" && false);
		return 0;
	}
private:
	LayoutElement() noexcept = default;
	LayoutElement(Type type) dbgexcept;

	std::string GetSignatureForStruct() const dbgexcept;
	std::string GetSignatureForArray() const dbgexcept;

	usize Finalize(usize offset) dbgexcept;
	usize FinalizeForStruct(usize offset) dbgexcept;
	usize FinalizeForArray(usize offset) dbgexcept;

	static LayoutElement& GetEmptyElement() noexcept;
	static bool CrossesBoundary(usize offset, usize size) noexcept;
	static usize AdvanceIfCrossesBoundary(usize offset, usize size) noexcept;
	static usize AdvanceToBoundary(usize offset) noexcept;
	static bool ValidateSymbolName(const std::string& name) noexcept;
private:
	std::optional<usize> offset;
	Type type = Empty;
	std::unique_ptr<ExtraDataBase> pExtraData;
};

class Layout {
	friend class LayoutCodex;
	friend class Buffer;
public:
	usize GetSizeInBytes() const noexcept;
	std::string GetSignature() const dbgexcept;
protected:
	Layout(std::shared_ptr<LayoutElement> pRoot) noexcept;
	std::shared_ptr<LayoutElement> pRoot;
};

class RawLayout : public Layout {
	friend class LayoutCodex;
public:
	RawLayout() noexcept;

	LayoutElement& operator[](const std::string& key) dbgexcept;

	LayoutElement& Add(Type type, const std::string& key) dbgexcept;
private:
	void ClearRoot() noexcept;
	std::shared_ptr<LayoutElement> DeliverRoot() noexcept;
};

class CookedLayout : public Layout {
	friend class LayoutCodex;
	friend class Buffer;
public:
	const LayoutElement& operator[](const std::string& key) const dbgexcept;
	std::shared_ptr<LayoutElement> ShareRoot() const noexcept;
private:
	CookedLayout(std::shared_ptr<LayoutElement> pRoot) noexcept;
	std::shared_ptr<LayoutElement> DeliverRoot() const noexcept;
};

class LayoutCodex {
public:
	static CookedLayout Resolve(RawLayout&& layout) dbgexcept;
private:
	static LayoutCodex& Get_() noexcept;
	std::unordered_map<std::string, std::shared_ptr<LayoutElement>> map;
};



class ConstElementRef {
	friend class Buffer;
	friend class ElementRef;
public:
	class Ptr {
		friend ConstElementRef;
	public:
		template<typename T>
		operator const T* () const dbgexcept {
			static_assert(ReverseInfo<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
			return &static_cast<const T&>(*ref);
		}
	private:
		Ptr(const ConstElementRef* ref) noexcept;
	private:
		const ConstElementRef* ref;
	};
public:
	bool Exists() const noexcept;

	ConstElementRef operator[](const std::string& key) const dbgexcept;
	ConstElementRef operator[](usize index) const dbgexcept;

	Ptr operator&() const dbgexcept;

	template<typename T>
	operator const T& () const dbgexcept {
		static_assert(ReverseInfo<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion");
		return *reinterpret_cast<const T*>(pBytes + offset + pLayout->Resolve<T>());
	}
private:
	ConstElementRef(const LayoutElement* pLayout, const u8* pBytes, usize offset) noexcept;
private:
	usize offset;
	const LayoutElement* pLayout;
	const u8* pBytes;
};

class ElementRef {
	friend class Buffer;
public:
	class Ptr {
		friend ElementRef;
	public:
		template<typename T>
		operator T* () const dbgexcept {
			static_assert(ReverseInfo<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
			return &static_cast<T&>(*ref);
		}
	private:
		Ptr(ElementRef* ref) noexcept;
	private:
		ElementRef* ref;
	};
public:
	operator ConstElementRef() const noexcept;

	bool Exists() const noexcept;
	
	ElementRef operator[](const std::string& key) const dbgexcept;
	ElementRef operator[](usize index) const dbgexcept;

	template<typename T>
	bool SetIfExists(const T& val) dbgexcept {
		if (Exists()) { *this = val; return true; } return false;
	}

	Ptr operator&() const dbgexcept;

	template<typename T>
	operator T&() const dbgexcept {
		static_assert(ReverseInfo<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion");
		return *reinterpret_cast<T*>(pBytes + offset + pLayout->Resolve<T>());
	}
	template<typename T>
	T& operator=(const T& rhs) const dbgexcept {
		static_assert(ReverseInfo<std::remove_const_t<T>>::valid, "Unsupported SysType used in assignment");
		return static_cast<T&>(*this) = rhs;
	}
private:
	ElementRef(const LayoutElement* pLayout, u8* pBytes, usize offset) noexcept;
private:
	usize offset;
	const LayoutElement* pLayout;
	u8* pBytes;
};



class Buffer {
public:
	Buffer(RawLayout&& layout) dbgexcept;
	Buffer(const CookedLayout& layout) dbgexcept;
	Buffer(CookedLayout&& layout) dbgexcept;
	Buffer(const Buffer& other) noexcept;
	Buffer(Buffer&& other) noexcept;

	ElementRef operator[](const std::string& key) dbgexcept;
	ConstElementRef operator[](const std::string& key) const dbgexcept;

	const u8* GetData() const noexcept;
	usize GetSizeInBytes() const noexcept;
	
	const LayoutElement& GetRootLayoutElement() const noexcept;
	std::shared_ptr<LayoutElement> ShareLayoutRoot() const noexcept;
	
	void CopyFrom(const Buffer& other) dbgexcept;
private:
	std::shared_ptr<LayoutElement> pLayoutRoot;
	std::vector<u8> bytes;
};

}

#ifndef CBD_IMPL
#undef BET
#undef BASE_ELEMENT_TYPES
#endif // !CBD_IMPL
