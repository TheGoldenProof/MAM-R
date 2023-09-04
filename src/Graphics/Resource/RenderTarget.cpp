#include "Graphics/GraphicsMacros.h"
#include "Graphics/Resource/DepthStencil.h"
#include "Graphics/Resource/RenderTarget.h"

namespace wrl = Microsoft::WRL;

RenderTarget::RenderTarget(Graphics& gfx, u32 width_, u32 height_) : width(width_), height(height_) {
	INFOMAN(gfx);

	D3D11_TEXTURE2D_DESC td{};
	td.Width = width;
	td.Height = height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;
	wrl::ComPtr<ID3D11Texture2D> pTexture;
	GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(&td, nullptr, &pTexture));

	D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
	sd.Format = td.Format;
	sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sd.Texture2D.MostDetailedMip = 0;
	sd.Texture2D.MipLevels = 1;
	GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(pTexture.Get(), &sd, &pTextureView));

	D3D11_RENDER_TARGET_VIEW_DESC rd{};
	rd.Format = td.Format;
	rd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rd.Texture2D = D3D11_TEX2D_RTV{0};
	GFX_THROW_INFO(GetDevice(gfx)->CreateRenderTargetView(pTexture.Get(), &rd, &pTargetView));
}

void RenderTarget::BindAsTexture(Graphics& gfx, u32 slot) const noexcept {
	GetContext(gfx)->PSSetShaderResources(slot, 1, pTextureView.GetAddressOf());
}

void RenderTarget::BindAsTarget(Graphics& gfx, const DepthStencil* pDepthStencil) const noexcept {
	GetContext(gfx)->OMSetRenderTargets(1, pTargetView.GetAddressOf(), pDepthStencil? pDepthStencil->pDepthStencilView.Get() : nullptr);

	D3D11_VIEWPORT vp;
	vp.Width = (f32)width;
	vp.Height = (f32)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	GetContext(gfx)->RSSetViewports(1, &vp);
}

void RenderTarget::BindAsTarget(Graphics& gfx, const DepthStencil& depthStencil) const noexcept {
	BindAsTarget(gfx, &depthStencil);
}

void RenderTarget::ClearBuffer(Graphics& gfx, f32 r, f32 g, f32 b) const noexcept {
	const f32 color[]{r, g, b, 1.0f};
	GetContext(gfx)->ClearRenderTargetView(pTargetView.Get(), color);
}

u32 RenderTarget::GetWidth() const noexcept { return width; }
u32 RenderTarget::GetHeight() const noexcept { return height; }
