#include "mamr_defs.h"
#include "Globe.h"
#include "Graphics/Bindable/Bindables.h"
#include "Graphics/Camera.h"
#include "Graphics\Graphics.h"
#include "Graphics/Render/FrameController.h"
#include "Graphics/Resource/RenderTarget.h"
#include "Util/DXUtil.h"

FrameController::FrameController(Graphics& gfx) {
	UNREFERENCED_PARAMETER(gfx);
	/*namespace dx = DirectX;

	Vtx::VertexLayout l;
	l.Append(Vtx::VertexLayout::Position2D);
	Vtx::VertexBuffer vbuf{l};
	vbuf.EmplaceBack(dx::XMFLOAT2{-1, 1});
	vbuf.EmplaceBack(dx::XMFLOAT2{ 1, 1});
	vbuf.EmplaceBack(dx::XMFLOAT2{-1,-1});
	vbuf.EmplaceBack(dx::XMFLOAT2{ 1,-1});
	pVbFull = VertexBuffer::Resolve(gfx, "$Full", std::move(vbuf));
	std::vector<u16> indicies{0,1,2,1,3,2};
	pIbFull = IndexBuffer::Resolve(gfx, "$Full", std::move(indicies));

	pVsFull = VertexShader::Resolve(gfx, "VS_Fullscreen.cso");
	pPsFull = PixelShader::Resolve(gfx, "PS_Texture.cso");
	pLayoutFull = InputLayout::Resolve(gfx, l, pVsFull->GetBytecode());*/
}

void FrameController::Accept(const QueueJob& job, usize target) noexcept {
	assert(target < passes.max_size());
	passes[target].Accept(job);
}

void FrameController::Execute(Globe& gb) dbgexcept {
	Graphics& gfx = gb.Gfx();

	//ds.Clear(gfx);
	
	gfx.BindSwapBuffer();
	//gfx.SetProjection(projF);
	for (u32 i = 0; i < 32; i++) passes[i].Execute(gfx);
}

void FrameController::Reset() noexcept {
	for (auto& p : passes) p.Reset();
}

Vtx::VertexLayout FrameController::MakeFullscreenQuadLayout() {
	Vtx::VertexLayout l;
	l.Append(Vtx::VertexLayout::Position2D);
	return l;
}