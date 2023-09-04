#include "Graphics/Bindable/TransformCBuf.h"
#include "Graphics/ConstantBufferDyn.h"

std::unique_ptr<VertexConstantBufferCaching> TransformCBuf::pVcbuf;

TransformCBuf::TransformCBuf(Graphics& gfx, u32 slot) {
	if (!pVcbuf) {
		CBD::RawLayout rl;
		rl.Add(CBD::Matrix, "modelView");
		rl.Add(CBD::Matrix, "modelViewProj");
		CBD::Buffer buf(std::move(rl));
		pVcbuf = std::make_unique<VertexConstantBufferCaching>(gfx, std::move(buf), slot);
	}
}

void TransformCBuf::Bind(Graphics& gfx) noexcept {
	UpdateBindImpl(gfx, GetTransforms(gfx));
}

void TransformCBuf::InitializeParentReference(const Drawable& parent) noexcept {
	pParent = &parent;
}

std::unique_ptr<BindableCloning> TransformCBuf::Clone() const noexcept {
	return std::make_unique<TransformCBuf>(*this);
}

void TransformCBuf::UpdateBindImpl(Graphics& gfx, const Transforms& tf) noexcept {
	assert(pParent != nullptr);
	pVcbuf->GetBuffer()["modelView"] = tf.modelView;
	pVcbuf->GetBuffer()["modelViewProj"] = tf.modelViewProj;
	//pVcbuf->Update(gfx);  // called by Bind
	pVcbuf->Bind(gfx);
}

TransformCBuf::Transforms TransformCBuf::GetTransforms(Graphics& gfx) noexcept {
	assert(pParent != nullptr);
	const auto modelView = pParent->GetTransformXM() * gfx.GetCamera();
	return {
		DirectX::XMMatrixTranspose(modelView),
		DirectX::XMMatrixMultiplyTranspose(modelView, gfx.GetProjection())
	};
}