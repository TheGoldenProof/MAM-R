#pragma once
#include "Bindable.h"

class Sampler : public Bindable {
public:
	Sampler(Graphics& gfx, D3D11_FILTER filter);
	void Bind(Graphics& gfx) noexcept override;

	static std::shared_ptr<Sampler> Resolve(Graphics& gfx, D3D11_FILTER filter);
	static std::string GenerateUID(D3D11_FILTER filter);
	std::string GetUID() const noexcept override;
protected:
	D3D11_FILTER filter;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
};