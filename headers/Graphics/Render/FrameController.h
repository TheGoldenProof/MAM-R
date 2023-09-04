#pragma once
#include "Graphics/Drawable/Quad.h"
#include "Graphics/Render/QueuePass.h"
#include "Graphics\Vertex.h"
#include <array>

class FrameController {
public:
	FrameController(Graphics& gfx);

	void Accept(const class QueueJob& job, usize target) noexcept;
	void Execute(class Globe& gb) dbgexcept;
	void Reset() noexcept;
private:
	static Vtx::VertexLayout MakeFullscreenQuadLayout();
private:
	std::array<QueuePass, 32> passes;
	//DepthStencil ds;
	//std::shared_ptr<class RenderTargetTexture> pRt0;

	//Quad qrt0;

	//DirectX::XMMATRIX proj0;

	/*std::shared_ptr<VertexBuffer> pVbFull;
	std::shared_ptr<IndexBuffer> pIbFull;
	std::shared_ptr<VertexShader> pVsFull;
	std::shared_ptr<PixelShader> pPsFull;
	std::shared_ptr<InputLayout> pLayoutFull;*/
};