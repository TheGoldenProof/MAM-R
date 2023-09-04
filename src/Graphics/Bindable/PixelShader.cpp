#include "Graphics/Bindable/PixelShader.h"
#include "Graphics/GraphicsMacros.h"
#include "Graphics/Bindable/BindableCodex.h"
#include "TGLib/Util.h"

PixelShader::PixelShader(Graphics& gfx, std::string const& path) {
	INFOMAN(gfx);

	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	GFX_THROW_INFO(D3DReadFileToBlob(TGLib::ToWide("shaders/" + path).c_str(), &pBlob));
	GFX_THROW_INFO(GetDevice(gfx)->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));
}

void PixelShader::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->PSSetShader(pPixelShader.Get(), nullptr, 0);
}

std::shared_ptr<PixelShader> PixelShader::Resolve(Graphics& gfx, const std::string& path) {
	return BindableCodex::Resolve<PixelShader>(gfx, path);
}

std::string PixelShader::GenerateUID(const std::string& path) {
	using namespace std::string_literals;
	return typeid(PixelShader).name() + "#"s + path;
}

std::string PixelShader::GetUID() const noexcept {
	return GenerateUID(path);
}
