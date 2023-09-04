#include "Graphics/Bindable/Rasterizer.h"
#include "Graphics/GraphicsMacros.h"
#include "Graphics/Bindable/BindableCodex.h"

Rasterizer::Rasterizer(Graphics& gfx, bool twoSided) : twoSided(twoSided) {
	INFOMAN(gfx);
	
	D3D11_RASTERIZER_DESC rd = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	rd.CullMode = twoSided? D3D11_CULL_NONE : D3D11_CULL_BACK;

	GFX_THROW_INFO(GetDevice(gfx)->CreateRasterizerState(&rd, &pRasterizer));
}

void Rasterizer::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->RSSetState(pRasterizer.Get());
}

std::shared_ptr<Rasterizer> Rasterizer::Resolve(Graphics& gfx, bool twoSided) {
	return BindableCodex::Resolve<Rasterizer>(gfx, twoSided);
}

std::string Rasterizer::GenerateUID(bool twoSided) {
	using namespace std::string_literals;
	return typeid(Rasterizer).name() + "#"s + (twoSided? "TwoSided":"OneSided");
}

std::string Rasterizer::GetUID() const noexcept {
	return GenerateUID(twoSided);
}
