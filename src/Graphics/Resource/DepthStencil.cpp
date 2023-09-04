#include "Graphics/Resource/DepthStencil.h"
#include "Graphics/GraphicsMacros.h"

namespace wrl = Microsoft::WRL;

DepthStencil::DepthStencil(Graphics& gfx, u32 width, u32 height) {
	INFOMAN(gfx);

	wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(&desc, nullptr, &pDepthStencil));

	GFX_THROW_INFO(GetDevice(gfx)->CreateDepthStencilView(pDepthStencil.Get(), nullptr, &pDepthStencilView));
}

void DepthStencil::BindDepthStencil(Graphics& gfx) const noexcept {
	GetContext(gfx)->OMSetRenderTargets(0, nullptr, pDepthStencilView.Get());
}

void DepthStencil::Clear(Graphics& gfx) const noexcept {
	GetContext(gfx)->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
