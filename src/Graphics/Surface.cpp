#include "Graphics/Surface.h"
#include "TGLib/TGLib_Util.h"
#include "Windows/Window.h"
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <sstream>

Surface::Exception::Exception(i32 line, const wchar_t* file, const std::wstring& note_, std::optional<HRESULT> hr) noexcept
	: MyException(line, TGLib::ToNarrow(file).c_str()), note(L"[Note] "+note_) {
	if (hr) note = L"[Error String] " + Window::Exception::TranslateErrorCode(*hr) + L"\n" + note;
}

Surface::Exception::Exception(i32 line, const wchar_t* file, const std::wstring& filename, const std::wstring& note_, std::optional<HRESULT> hr) noexcept
	: MyException(line, TGLib::ToNarrow(file).c_str()) {
	using namespace std::string_literals;
	note = L"[File] "s + filename + L"\n" + L"[Note] " + note_;
	if (hr) note = L"[Error String] " + Window::Exception::TranslateErrorCode(*hr) + L"\n" + note;
}

const char* Surface::Exception::what() const noexcept {
	std::ostringstream oss;
	oss << MyException::what() << std::endl << TGLib::ToNarrow(GetNote());
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Surface::Exception::GetType() const noexcept { return "Surface Exception"; }

const std::wstring& Surface::Exception::GetNote() const noexcept { return note; }

Surface::Surface(u32 width, u32 height) : isError(false) {
	HRESULT hr = scratch.Initialize2D(format, width, height, 1, 1);
	if (FAILED(hr)) throw Exception(__LINE__, __FILEW__, L"Failed to initialize ScratchImage", hr);
}

Surface::Surface(DirectX::ScratchImage scratch, bool isError) noexcept
	: scratch(std::move(scratch)), isError(isError) {} 

void Surface::Clear(Color fill) noexcept {
	//const u32 width = GetWidth();
	const u32 height = GetHeight();
	auto& imgData = *scratch.GetImage(0, 0, 0);
	for (usize y = 0; y < height; y++) {
		Color* rowStart = (Color*)(imgData.pixels + imgData.rowPitch*y);
		std::fill(rowStart, rowStart + imgData.width, fill);
	}
}

void Surface::SetPixel(u32 x, u32 y, Color c) dbgexcept {
	assert(x >= 0);
	assert(y >= 0);
	assert(x < GetWidth());
	assert(y < GetHeight());
	auto& imgData = *scratch.GetImage(0, 0, 0);
	((Color*)(imgData.pixels + imgData.rowPitch*y))[x] = c;
}

Surface::Color Surface::GetPixel(u32 x, u32 y) dbgexcept {
	assert(x >= 0);
	assert(y >= 0);
	assert(x < GetWidth());
	assert(y < GetHeight());
	auto& imgData = *scratch.GetImage(0, 0, 0);
	return ((Color*)(imgData.pixels + imgData.rowPitch*y))[x];
}

u32 Surface::GetWidth() const noexcept { return (u32)scratch.GetMetadata().width; }
u32 Surface::GetHeight() const noexcept { return (u32)scratch.GetMetadata().height; }

Surface::Color* Surface::GetBufferPtr() noexcept { return (Color*)(scratch.GetPixels()); }
const Surface::Color* Surface::GetBufferPtr() const noexcept { return const_cast<Surface*>(this)->GetBufferPtr(); }
const Surface::Color* Surface::GetBufferPtrConst() const noexcept { return const_cast<Surface*>(this)->GetBufferPtr(); }

Surface Surface::FromFile(const std::wstring& name) {
	DirectX::ScratchImage scratch;
	HRESULT hr = DirectX::LoadFromWICFile(name.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, scratch);
	if (FAILED(hr)) {
		Surface::Exception e(__LINE__, __FILEW__, name, L"Failed to load image", hr);
		DEBUG_LOG("\n\n");
		DEBUG_LOG(e.what());
		DEBUG_LOG("\n\n");

		return ErrorTexture();
	}

	if (scratch.GetImage(0, 0, 0)->format != format) {
		DirectX::ScratchImage converted;
		hr = DirectX::Convert(*scratch.GetImage(0, 0, 0), format, DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, converted);
		if (FAILED(hr)) throw Exception(__LINE__, __FILEW__, name, L"Failed to convert image", hr);
		return Surface(std::move(converted));
	}

	return Surface(std::move(scratch));
}

Surface Surface::ErrorTexture() {
	constexpr u32 magenta = 0xffff00ff;
	constexpr u32 black = 0xff00ff00;

	DirectX::ScratchImage errorTex;
	HRESULT hr = errorTex.Initialize2D(format, 2, 2, 1, 1);
	if (FAILED(hr)) throw Exception(__LINE__, __FILEW__, L"Failed to initialize ScratchImage", hr);

	auto& data = *errorTex.GetImage(0, 0, 0);
	((u32*)(data.pixels + data.rowPitch * 0))[0] = magenta;
	((u32*)(data.pixels + data.rowPitch * 1))[0] = black;
	((u32*)(data.pixels + data.rowPitch * 0))[1] = black;
	((u32*)(data.pixels + data.rowPitch * 1))[1] = magenta;

	return Surface(std::move(errorTex), true);
}

void Surface::Save(std::wstring const& filename) const {
	const auto GetCodecID = [](const std::wstring& filename) {
		const std::filesystem::path path = filename;
		const auto ext = path.extension().wstring();
		#define CASE(str, codec) if (ext == str) { return codec; }
		CASE(L".png", DirectX::WIC_CODEC_PNG) else
		CASE(L".gif", DirectX::WIC_CODEC_GIF) else
		CASE(L".bmp", DirectX::WIC_CODEC_BMP) else
		CASE(L".ico", DirectX::WIC_CODEC_ICO) else
		CASE(L".jpg" || ext == L".jpeg", DirectX::WIC_CODEC_JPEG) else
		CASE(L".tif" || ext == L".tiff", DirectX::WIC_CODEC_TIFF) else
		CASE(L".heif" || ext == L".heic", DirectX::WIC_CODEC_HEIF) else
		CASE(L".hdp" || ext == L".wdp" || ext == L".jxr", DirectX::WIC_CODEC_WMP)
		#undef CASE
		else throw Exception(__LINE__, __FILEW__, filename, L"Image format not supported");
	};

	HRESULT hr = DirectX::SaveToWICFile(*scratch.GetImage(0, 0, 0), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(GetCodecID(filename)), filename.c_str());
	if (FAILED(hr)) throw Exception(__LINE__, __FILEW__, filename, L"Failed to save image", hr);
}

bool Surface::HasAlpha() const noexcept {
	return !scratch.IsAlphaAllOpaque();
}

bool Surface::IsErrorTex() const noexcept {
	return isError;
}
