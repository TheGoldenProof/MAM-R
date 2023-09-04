#pragma once
#include "Graphics/Bindable/Bindable.h"
#include "Graphics/Vertex.h"

class InputLayout : public Bindable {
public:
	InputLayout(Graphics& gfx, Vtx::VertexLayout layout_in, ID3DBlob* pVertexShaderBytecode);
	void Bind(Graphics& gfx) noexcept override;

	const Vtx::VertexLayout GetLayout() const noexcept;
	static std::shared_ptr<InputLayout> Resolve(Graphics& gfx, const Vtx::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode);
	static std::string GenerateUID(const Vtx::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode = nullptr);
	std::string GetUID() const noexcept override;
protected:
	Vtx::VertexLayout layout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
};