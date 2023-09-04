#include "Graphics/Bindable/Sampler.h"
#include "Graphics/GraphicsMacros.h"
#include "Graphics/Bindable/BindableCodex.h"

Sampler::Sampler(Graphics& gfx, D3D11_FILTER filter_) : filter(filter_) {
	INFOMAN(gfx);

	D3D11_SAMPLER_DESC sd{};
	sd.Filter = filter;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
	sd.MipLODBias = 0;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	GFX_THROW_INFO(GetDevice(gfx)->CreateSamplerState(&sd, &pSampler));
}

void Sampler::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->PSSetSamplers(0, 1, pSampler.GetAddressOf());
}

std::shared_ptr<Sampler> Sampler::Resolve(Graphics& gfx, D3D11_FILTER filter_) {
	return BindableCodex::Resolve<Sampler>(gfx, filter_);
}

std::string Sampler::GenerateUID(D3D11_FILTER filter_) {
	using namespace std::string_literals;
	return typeid(Sampler).name() + ",f="s + std::to_string(filter_);
}

std::string Sampler::GetUID() const noexcept {
	return GenerateUID(filter);
}
