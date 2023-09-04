#pragma once
#include "Graphics/Graphics.h"
#include "Util\Misc.h"
#include <utility>
#include <vector>
#include <type_traits>

#define LAYOUT_ELEMENT_TYPES \
	X(Position2D) \
	X(Position3D) \
	X(Texture2D) \
	X(Normal) \
	X(Tangent) \
	X(Bitangent) \
	X(Float3Color) \
	X(Float4Color) \
	X(Byte4Color) \
	X(Count)

namespace Vtx {

struct Byte4Color {
	u8 a, r, g, b;
};

class VertexLayout {
public:
	enum ElementType {
		#define X(x) x,
		LAYOUT_ELEMENT_TYPES
		#undef X
	};

	template<ElementType> struct Info /*{
		using SysType = f64;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
		static constexpr const char* semantic = "!INVALID!";
		static constexpr const char* code = "!INV!";
	}*/;
	template<> struct Info<Position2D> {
		using SysType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char* semantic = "Position";
		static constexpr const char* code = "P2";
	};
	template<> struct Info<Position3D> {
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Position";
		static constexpr const char* code = "P3";
	};
	template<> struct Info<Texture2D> {
		using SysType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char* semantic = "Texcoord";
		static constexpr const char* code = "T2";
	};
	template<> struct Info<Normal> {
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Normal";
		static constexpr const char* code = "N";
	};
	template<> struct Info<Tangent> {
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Tangent";
		static constexpr const char* code = "Nt";
	};
	template<> struct Info<Bitangent> {
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Bitangent";
		static constexpr const char* code = "Nb";
	};
	template<> struct Info<Float3Color> {
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Color";
		static constexpr const char* code = "C3";
	};
	template<> struct Info<Float4Color> {
		using SysType = DirectX::XMFLOAT4;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		static constexpr const char* semantic = "Color";
		static constexpr const char* code = "C4";
	};
	template<> struct Info<Byte4Color> {
		using SysType = Vtx::Byte4Color;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr const char* semantic = "Color";
		static constexpr const char* code = "C8";
	};
	template<> struct Info<Count> {
		using SysType = f64;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
		static constexpr const char* semantic = "!INVALID!";
		static constexpr const char* code = "!INV!";
	};

	template<template<VertexLayout::ElementType> class F, typename... Args>
	static constexpr auto Bridge(VertexLayout::ElementType type, Args&&... args) dbgexcept {
		switch (type) {
			#define X(x) case VertexLayout::x: return F<VertexLayout::x>::Exec(std::forward<Args>(args)...);
			LAYOUT_ELEMENT_TYPES
			#undef X
		}
		assert("Invalid element type" && false);
		return F<VertexLayout::Count>::Exec(std::forward<Args>(args)...);
	}

	class Element {
	public:
		static constexpr usize SizeOf(ElementType type) dbgexcept;

		Element(ElementType type, usize offset);
		usize GetOffsetAfter() const dbgexcept;
		usize GetOffset() const dbgexcept;
		usize Size() const dbgexcept;
		ElementType GetType() const noexcept;
		D3D11_INPUT_ELEMENT_DESC GetDesc() const dbgexcept;
		const char* GetCode() const noexcept;

		bool operator==(const Element& other);
	private:
		template<ElementType type>
		static constexpr D3D11_INPUT_ELEMENT_DESC GenerateDesc(usize offset) dbgexcept {
			return {Info<type>::semantic, 0, Info<type>::dxgiFormat, 0, (u32)offset, D3D11_INPUT_PER_VERTEX_DATA, 0};
		}
	private:
		ElementType type;
		usize offset;
	};
public:
	template<ElementType Type>
	Element const& Resolve() const dbgexcept {
		for (auto const& e : elements)
			if (e.GetType() == Type) return e;
		assert("Could not resolve element type" && false);
		return elements.front();
	}
	const Element& ResolveByIndex(usize i) const dbgexcept;
	usize Size() const dbgexcept;
	VertexLayout& Append(ElementType type) dbgexcept;
	usize GetElementCount() const noexcept;
	std::vector<D3D11_INPUT_ELEMENT_DESC> GetD3DLayout() const dbgexcept;
	std::string GetCode() const dbgexcept;
	bool Has(ElementType type) const noexcept;
	bool operator==(const VertexLayout& other);
private:
	std::vector<Element> elements;
};

class Vertex {
	friend class VertexBuffer;
private:
	template<VertexLayout::ElementType Type>
	struct AttributeSetting {
		template<typename T>
		static constexpr auto Exec(Vertex* pVertex, u8* pAttribute, T&& val) dbgexcept {
			return pVertex->SetAttribute<Type>(pAttribute, std::forward<T>(val));
		}
	};
public:
	template<VertexLayout::ElementType Type>
	auto& Attr() dbgexcept {
		auto pAttribute = pData + layout.Resolve<Type>().GetOffset();
		return *reinterpret_cast<typename VertexLayout::Info<Type>::SysType*>(pAttribute);
	}
	template<typename T>
	void SetAttributeByIndex(usize i, T&& val) dbgexcept {
		const auto& element = layout.ResolveByIndex(i);
		auto pAttribute = pData + element.GetOffset();
		VertexLayout::Bridge<AttributeSetting>(element.GetType(), this, pAttribute, std::forward<T>(val));
	}
protected:
	Vertex(u8* pData, const VertexLayout& layout) dbgexcept;
private:
	template<typename First, typename... Rest>
	void SetAttributeByIndex(usize i, First&& first, Rest&&... rest) dbgexcept {
		SetAttributeByIndex(i, std::forward<First>(first));
		SetAttributeByIndex(i+1, std::forward<Rest>(rest)...);
	}
	template<VertexLayout::ElementType DestLayoutType, typename SrcType>
	void SetAttribute(u8* pAttribute, SrcType&& val) dbgexcept {
		using Dest = typename VertexLayout::Info<DestLayoutType>::SysType;
		if constexpr (std::is_assignable_v<Dest, SrcType>) {
			*reinterpret_cast<Dest*>(pAttribute) = val;
		} else {
			assert("Parameter attribute type mismatch" && false);
		}
	}
private:
	u8* pData = nullptr;
	const VertexLayout& layout;
};

class ConstVertex {
public:
	ConstVertex(const Vertex& v) dbgexcept;
	template<VertexLayout::ElementType Type>
	const auto& Attr() const dbgexcept { return const_cast<Vertex&>(vertex).Attr<Type>(); }
private:
	Vertex vertex;
};

class VertexBuffer {
public:
	VertexBuffer() {}
	VertexBuffer(VertexLayout layout, usize size = 0) dbgexcept;
	const u8* GetData() const dbgexcept;
	const VertexLayout& GetLayout() const noexcept;
	usize Size() const dbgexcept;
	usize SizeBytes() const noexcept { return buffer.size(); }
	void Resize(usize newSize) dbgexcept;
	void Reserve(usize newCapacity);
	void Clear();
	template<typename... Params>
	void EmplaceBack(Params&&... params) dbgexcept {
		assert(sizeof...(params) == layout.GetElementCount() && "Param count doesn't match number of vertex elements");
		buffer.resize(buffer.size() + layout.Size());
		Back().SetAttributeByIndex<Params...>(0, std::forward<Params>(params)...);
	}
	Vertex Back() dbgexcept;
	Vertex Front() dbgexcept;
	Vertex operator[](usize i) dbgexcept;
	ConstVertex Back() const dbgexcept;
	ConstVertex Front() const dbgexcept;
	ConstVertex operator[](usize i) const dbgexcept;
private:
	std::vector<u8> buffer;
	VertexLayout layout;
};
}

MAKE_HASHABLE(Vtx::VertexLayout::Element, t.GetType(), t.GetOffset());
MAKE_HASHABLE(D3D11_INPUT_ELEMENT_DESC,
	std::string(t.SemanticName, std::strlen(t.SemanticName)),
	t.SemanticIndex, t.Format, t.InputSlot, t.AlignedByteOffset, t.InputSlotClass, t.InstanceDataStepRate);
namespace std {
template<> struct hash<Vtx::VertexLayout> {
	size_t operator()(const Vtx::VertexLayout& vl) const {
		const auto& d3dLayout = vl.GetD3DLayout();
		return hash_range(d3dLayout.begin(), d3dLayout.end());
	}
};
}
MAKE_HASHABLE(Vtx::VertexBuffer, 
	hash_range(t.GetData(), t.GetData()+t.SizeBytes()),
	t.GetLayout())

#ifndef VTX_IMPL
#undef LAYOUT_ELEMENT_TYPES
#endif // !VTX_IMPL
