#pragma once
#include "Graphics/Bindable/ConstantBuffers.h"
#include "Graphics/Drawable/Drawable.h"

class TransformCBuf : public BindableCloning {
protected:
	struct Transforms {
		DirectX::XMMATRIX modelView;
		DirectX::XMMATRIX modelViewProj;
	};
public:
	TransformCBuf(Graphics& gfx, u32 slot = 0);

	void Bind(Graphics& gfx) noexcept override;
	void InitializeParentReference(const Drawable& parent) noexcept override;
	std::unique_ptr<BindableCloning> Clone() const noexcept override;
	std::string GetUID() const noexcept override { return typeid(TransformCBuf).name(); };
protected:
	void UpdateBindImpl(Graphics& gfx, const Transforms& tf) noexcept;
	Transforms GetTransforms(Graphics& gfx) noexcept;
private:
	static std::unique_ptr<VertexConstantBufferCaching> pVcbuf;
	const Drawable* pParent = nullptr;
};