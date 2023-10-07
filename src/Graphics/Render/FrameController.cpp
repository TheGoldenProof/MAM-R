#include "omidiv_defs.h"
#include "Globe.h"
#include "Graphics/Bindable/Bindables.h"
#include "Graphics/Camera.h"
#include "Graphics\Graphics.h"
#include "Graphics/Render/FrameController.h"
#include "Graphics/Resource/RenderTarget.h"
#include "Util/DXUtil.h"
#include "Util\MyMath.h"
#include <algorithm>

FrameController::FrameController(Graphics& gfx) {
	UNREFERENCED_PARAMETER(gfx);
	fov = Math::to_rad(fov);
	/*namespace dx = DirectX;

	Vtx::VertexLayout l;
	l.Append(Vtx::VertexLayout::Position2D);
	Vtx::VertexBuffer vbuf{l};
	vbuf.EmplaceBack(dx::XMFLOAT2{-1, 1});
	vbuf.EmplaceBack(dx::XMFLOAT2{ 1, 1});
	vbuf.EmplaceBack(dx::XMFLOAT2{-1,-1});
	vbuf.EmplaceBack(dx::XMFLOAT2{ 1,-1});
	pVbFull = VertexBuffer::Resolve(gfx, "$Full", std::move(vbuf));
	std::vector<u32> indicies{0,1,2,1,3,2};
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
	fov = std::clamp(fov, 0.0078125f, Math::PI<f32>-0.0078125f);

	const auto currentCamName = gb.Cams().GetActiveCamera().transform([](const auto& camRef) { return camRef.get().GetName(); });
	const bool depthTestEnabled = gfx.GetDepthTestEnabled();

	constexpr f32 nearZ = 1.0f / 64;
	const f32 aspect = static_cast<f32>(gfx.GetWidth()) / static_cast<f32>(gfx.GetHeight());
	auto proj = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, 8192.0f);
	gb.Gfx().SetProjection(std::move(proj));
	
	gfx.BindSwapBuffer();
	//gfx.SetProjection(projF);
	for (u32 i = 0; i < 16; i++) passes[i].Execute(gfx);

	gb.Cams().SetActive("Camera1");
	gb.Cams().BindActive(gfx);
	//f32 projW = OMIDIV_WINW;
	//const f32 projH = OMIDIV_WINH;
	//if (aspect != static_cast<f32>(OMIDIV_WINW) / OMIDIV_WINH) {
	//	projW = projH * aspect;
	//}
	//const f32 scale = 1.0f;// tan(fov / 2.0f);
	//gfx.SetProjection(DXUtil::CustomOrthoProj(projW * scale, projH * scale));
	gfx.SetProjection(DXUtil::CustomOrthoProj(aspect, 1.0f));
	//gfx.SetProjection(DXUtil::CustomOrthoProj(OMIDIV_WINW, OMIDIV_WINH));
	for (u32 i = 16; i < 24; i++) passes[i].Execute(gfx);

	gb.Cams().SetActive("HUD cam");
	gb.Cams().BindActive(gfx);
	gfx.SetDepthTest(true, D3D11_COMPARISON_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO);
	passes[24].Execute(gfx);

	//gfx.SetProjection(DXUtil::CustomOrthoProj(static_cast<f32>(gfx.GetWidth()), static_cast<f32>(gfx.GetHeight())));
	gfx.SetDepthTest(false);
	for (u32 i = 25; i < 32; i++) passes[i].Execute(gfx);

	if (currentCamName) gb.Cams().SetActive(currentCamName.value());
	if (depthTestEnabled) gfx.SetDepthTest(true);
}

void FrameController::Reset() noexcept {
	for (auto& p : passes) p.Reset();
}

Vtx::VertexLayout FrameController::MakeFullscreenQuadLayout() {
	Vtx::VertexLayout l;
	l.Append(Vtx::VertexLayout::Position2D);
	return l;
}
