#pragma once
#include "Graphics/Bindable/Bindable.h"
#include <array>
#include <optional>

class Blender : public Bindable {
public:
	Blender(Graphics& gfx, bool blending, std::optional<f32> factor = {});

	void SetFactor(f32 factor) dbgexcept;
	f32 GetFactor() const dbgexcept;

	void Bind(Graphics& gfx) noexcept override;
	static std::shared_ptr<Blender> Resolve(Graphics& gfx, bool blending, std::optional<f32> factor);
	static std::string GenerateUID(bool blending, std::optional<f32> factor);
	std::string GetUID() const noexcept override;
protected:
	Microsoft::WRL::ComPtr<ID3D11BlendState> pBlender;
	bool blending;
	std::optional<std::array<f32, 4>> factors;
};