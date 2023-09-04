#pragma once
#include "Graphics/Resource/GraphicsResource.h"

class DepthStencil : public GraphicsResource {
	friend class RenderTarget;
	friend class Graphics;
public:
	DepthStencil(Graphics& gfx, u32 width, u32 height);
	void BindDepthStencil(Graphics& gfx) const noexcept;
	void Clear(Graphics& gfx) const noexcept;
private:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
};