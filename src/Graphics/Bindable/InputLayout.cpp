#include "Graphics/Bindable/InputLayout.h"
#include "Graphics/GraphicsMacros.h"
#include "Graphics/Bindable/BindableCodex.h"

InputLayout::InputLayout(Graphics& gfx, Vtx::VertexLayout layout_in, ID3DBlob* pVertexShaderBytecode) 
	: layout(std::move(layout_in)) {
	INFOMAN(gfx);

	const auto d3dLayout = layout.GetD3DLayout();

	GFX_THROW_INFO(GetDevice(gfx)->CreateInputLayout(
		d3dLayout.data(), (u32)d3dLayout.size(),
		pVertexShaderBytecode->GetBufferPointer(),
		pVertexShaderBytecode->GetBufferSize(),
		&pInputLayout
	));
}

void InputLayout::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->IASetInputLayout(pInputLayout.Get());
}

const Vtx::VertexLayout InputLayout::GetLayout() const noexcept { return layout; }

std::shared_ptr<InputLayout> InputLayout::Resolve(Graphics& gfx, const Vtx::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode) {
	return BindableCodex::Resolve<InputLayout>(gfx, layout, pVertexShaderBytecode);
}

std::string InputLayout::GenerateUID(const Vtx::VertexLayout& layout, ID3DBlob* pVertexShaderBytecode) {
	UNREFERENCED_PARAMETER(pVertexShaderBytecode);
	using namespace std::string_literals;
	return typeid(InputLayout).name() + "#"s + layout.GetCode();
}

std::string InputLayout::GetUID() const noexcept {
	return GenerateUID(layout);
}