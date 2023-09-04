#pragma once
#include "Graphics/Resource/GraphicsResource.h"

class DepthStencil;

class RenderTarget : public GraphicsResource {
public:
	RenderTarget(Graphics& gfx, u32 width, u32 height);
	void BindAsTexture(Graphics& gfx, u32 slot) const noexcept;
	void BindAsTarget(Graphics& gfx, const DepthStencil* pDepthStencil = nullptr) const noexcept;
	void BindAsTarget(Graphics& gfx, const DepthStencil& depthStencil) const noexcept;
	void ClearBuffer(Graphics& gfx, f32 r, f32 g, f32 b) const noexcept;

	u32 GetWidth() const noexcept;
	u32 GetHeight() const noexcept;
protected:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTargetView;

	u32 width;
	u32 height;
};