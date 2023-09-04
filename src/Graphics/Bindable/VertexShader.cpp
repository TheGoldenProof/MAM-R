#include "Graphics/Bindable/VertexShader.h"
#include "Graphics/GraphicsMacros.h"
#include "Graphics/Bindable/BindableCodex.h"
#include "TGLib/Util.h"

VertexShader::VertexShader(Graphics& gfx, std::string const& path) : path(path) {
	INFOMAN(gfx);

	GFX_THROW_INFO(D3DReadFileToBlob(TGLib::ToWide("shaders/" + path).c_str(), &pBytecodeBlob));
	GFX_THROW_INFO(GetDevice(gfx)->CreateVertexShader(
		pBytecodeBlob->GetBufferPointer(),
		pBytecodeBlob->GetBufferSize(),
		nullptr, &pVertexShader
	));
}

ID3DBlob* VertexShader::GetBytecode() const noexcept { return pBytecodeBlob.Get(); }

void VertexShader::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->VSSetShader(pVertexShader.Get(), nullptr, 0);
}

std::shared_ptr<VertexShader> VertexShader::Resolve(Graphics& gfx, const std::string& path) {
	return BindableCodex::Resolve<VertexShader>(gfx, path);
}

std::string VertexShader::GenerateUID(const std::string& path) {
	using namespace std::string_literals;
	return typeid(VertexShader).name() + "#"s + path;
}

std::string VertexShader::GetUID() const noexcept {
	return GenerateUID(path);
}
