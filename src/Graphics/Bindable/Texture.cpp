#include "Graphics/Bindable/Texture.h"
#include "Graphics/Bindable/BindableCodex.h"
#include "Graphics/GraphicsMacros.h"
#include "TGLib\TGLib_Util.h"

namespace wrl = Microsoft::WRL;

Texture::Texture(Graphics& gfx, const std::wstring& path, u32 slot) : path(path), slot(slot) {
	UNREFERENCED_PARAMETER(gfx);
}

void Texture::UnbindTexture(Graphics& gfx, u32 slot) noexcept {
	ID3D11ShaderResourceView* const pNullSRV[] = { nullptr };
	GetContext(gfx)->PSSetShaderResources(slot, 1, pNullSRV);
}

#pragma region SurfaceTexture

SurfaceTexture::SurfaceTexture(Graphics& gfx, const std::wstring& path, u32 slot) 
	: SurfaceTexture(gfx, Surface::FromFile(path), path, slot) {}

SurfaceTexture::SurfaceTexture(Graphics& gfx, Surface&& s, const std::wstring& name, u32 slot)
	: Texture(gfx, name, slot), surface(std::move(s)) {
	INFOMAN(gfx);

	D3D11_TEXTURE2D_DESC td{};
	td.Width = surface.GetWidth();
	td.Height = surface.GetHeight();
	td.MipLevels = 0;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	td.CPUAccessFlags = 0;
	td.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	D3D11_SUBRESOURCE_DATA srd{};
	srd.pSysMem = surface.GetBufferPtr();
	srd.SysMemPitch = surface.GetWidth() * sizeof(Surface::Color);
	wrl::ComPtr<ID3D11Texture2D> pTexture;
	GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(&td, nullptr, &pTexture));

	GetContext(gfx)->UpdateSubresource(pTexture.Get(), 0, nullptr, surface.GetBufferPtrConst(), surface.GetWidth()*sizeof(Surface::Color), 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
	srvd.Format = td.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MostDetailedMip = 0;
	srvd.Texture2D.MipLevels = (u32)-1;
	GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(pTexture.Get(), &srvd, &pTextureView));

	GetContext(gfx)->GenerateMips(pTextureView.Get());
}

Surface& SurfaceTexture::GetSurface() noexcept { return surface; }

u32 SurfaceTexture::GetWidth() const noexcept { return surface.GetWidth(); }
u32 SurfaceTexture::GetHeight() const noexcept { return surface.GetHeight(); }

bool SurfaceTexture::IsErrored() const noexcept {
	return surface.IsErrorTex();
}

void SurfaceTexture::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->PSSetShaderResources(0, 1, pTextureView.GetAddressOf());
}

std::shared_ptr<SurfaceTexture> SurfaceTexture::Resolve(Graphics& gfx, const std::wstring& path, u32 slot) {
	return BindableCodex::Resolve<SurfaceTexture>(gfx, path, slot);
}

std::string SurfaceTexture::GenerateUID(const std::wstring& path, u32 slot) {
	using namespace std::string_literals;
	return typeid(Texture).name() + "#"s + TGLib::ToNarrow(path) + "#" + std::to_string(slot);
}

std::string SurfaceTexture::GetUID() const noexcept {
	return GenerateUID(path, slot);
}
#pragma endregion

#pragma region RenderTargetTexture

RenderTargetTexture::RenderTargetTexture(Graphics& gfx, u32 width, u32 height, const std::wstring& name, u32 slot) 
	: Texture(gfx, name, slot), RenderTarget(gfx, width, height) {}

RenderTargetTexture::RenderTargetTexture(Graphics& gfx, const std::wstring& name, u32 slot) 
	: RenderTargetTexture(gfx, 128, 128, name, slot) {}

RenderTarget& RenderTargetTexture::GetRenderTarget() noexcept {
	return *this;
}

u32 RenderTargetTexture::GetWidth() const noexcept { return RenderTarget::GetWidth(); }
u32 RenderTargetTexture::GetHeight() const noexcept { return RenderTarget::GetHeight(); }

void RenderTargetTexture::Bind(Graphics& gfx) noexcept {
	BindAsTexture(gfx, slot);
}

std::shared_ptr<RenderTargetTexture> RenderTargetTexture::Resolve(Graphics& gfx, u32 width, u32 height, const std::wstring& name, u32 slot) {
	return BindableCodex::Resolve<RenderTargetTexture>(gfx, width, height, name, slot);
}

std::shared_ptr<RenderTargetTexture> RenderTargetTexture::Resolve(Graphics& gfx, const std::wstring& name, u32 slot) {
	return BindableCodex::Resolve<RenderTargetTexture>(gfx, name, slot);
}

std::string RenderTargetTexture::GenerateUID(u32 width, u32 height, const std::wstring& name, u32 slot) {
	UNREFERENCED_PARAMETER(width);
	UNREFERENCED_PARAMETER(height);
	return GenerateUID(name, slot);
}

std::string RenderTargetTexture::GenerateUID(const std::wstring& name, u32 slot) {
	using namespace std::string_literals;
	return typeid(RenderTargetTexture).name() + "#"s + TGLib::ToNarrow(name) + "#" + std::to_string(slot);
}

std::string RenderTargetTexture::GetUID() const noexcept {
	return GenerateUID(width, height, path, slot);
}
#pragma endregion
