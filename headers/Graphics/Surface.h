#pragma once
#include "Windows/framework.h"
#include "MyException.h"
#include "DXTex/DirectXTex.h"
#include <string>
#include <optional>

class Surface {
public:
	union Color {
		#pragma warning(push)
		#pragma warning(disable: 4201 26945)
		u32 dword = 0;
		struct {
			u8 a;
			u8 r;
			u8 g;
			u8 b;
		};

		constexpr Color() noexcept : dword() {}
		constexpr Color(Color const& col) noexcept : dword(col.dword) {}
		constexpr Color(u32 dw) noexcept : dword(dw) {}
		constexpr Color(u8 a, u8 r, u8 g, u8 b) noexcept : a(a), r(r), g(g), b(b) {}
		constexpr Color(u8 r, u8 g, u8 b) noexcept : a(255), r(r), g(g), b(b) {}
		constexpr Color(Color const& col, u8 a) noexcept : a(a), r(col.r), g(col.g), b(col.b) {}

		#pragma warning(pop)

		Color& operator=(Color const& color) noexcept { dword = color.dword; return *this; }
		constexpr u8 GetA() const noexcept { return a; }
		constexpr u8 GetR() const noexcept { return r; }
		constexpr u8 GetG() const noexcept { return g; }
		constexpr u8 GetB() const noexcept { return b; }
		void SetA(u8 a_) noexcept { a = a_; }
		void SetR(u8 r_) noexcept { r = r_; }
		void SetG(u8 g_) noexcept { g = g_; }
		void SetB(u8 b_) noexcept { b = b_; }
	};

	class Exception : public MyException {
	public:
		Exception(i32 line, const char* file, const std::string& note, std::optional<HRESULT> hr ={}) noexcept;
		Exception(i32 line, const char* file, const std::string& filename, const std::string& note, std::optional<HRESULT> hr ={}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		const std::string& GetNote() const noexcept;
	private:
		std::optional<HRESULT> hr;
		std::string note;
	};
public:
	Surface(u32 width, u32 height);
	Surface(Surface&& source) noexcept = default;
	Surface(Surface const&) = delete;
	Surface& operator=(Surface&& source) noexcept = default;
	Surface& operator=(Surface const&) = delete;
	~Surface() = default;

	void Clear(Color fill) noexcept;
	void SetPixel(u32 x, u32 y, Color c) dbgexcept;
	Color GetPixel(u32 x, u32 y) dbgexcept;
	u32 GetWidth() const noexcept;
	u32 GetHeight() const noexcept;
	Color* GetBufferPtr() noexcept;
	const Color* GetBufferPtr() const noexcept;
	const Color* GetBufferPtrConst() const noexcept;
	
	static Surface FromFile(std::string const& name);
	static Surface ErrorTexture();
	void Save(std::string const& filename) const;
	bool HasAlpha() const noexcept;
	bool IsErrorTex() const noexcept;
private:
	Surface(DirectX::ScratchImage scratch, bool isError = false) noexcept;
private:
	static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	DirectX::ScratchImage scratch;
	bool isError;
};