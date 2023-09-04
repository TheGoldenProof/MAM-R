#include "Graphics/Bindable/Blender.h"
#include "Graphics/GraphicsMacros.h"
#include "Graphics/Bindable/BindableCodex.h"

Blender::Blender(Graphics& gfx, bool blending, std::optional<f32> factor) : blending(blending) {
	INFOMAN(gfx);

	if (factor) {
		factors.emplace();
		factors->fill(*factor);
	}

	D3D11_BLEND_DESC bd = CD3D11_BLEND_DESC(CD3D11_DEFAULT{});
	auto& brt = bd.RenderTarget[0];
	if (blending) {
		brt.BlendEnable = TRUE;
		if (factor) {
			brt.SrcBlend = D3D11_BLEND_BLEND_FACTOR;
			brt.DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
		} else {
			brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		}
		brt.BlendOp = D3D11_BLEND_OP_ADD;
		brt.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		brt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		brt.BlendOpAlpha = D3D11_BLEND_OP_ADD;

		for (u32 i = 1; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) {
			bd.RenderTarget[i] = brt;
		}
	}

	GFX_THROW_INFO(GetDevice(gfx)->CreateBlendState(&bd, &pBlender));
}

void Blender::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->OMSetBlendState(pBlender.Get(), factors? factors->data() : nullptr, 0xFFFFFFFF);
}

void Blender::SetFactor(f32 factor) dbgexcept {
	assert(factors && "Blender must be created with a factor in order to change it.");
	factors->fill(factor);
}

f32 Blender::GetFactor() const dbgexcept {
	assert(factors && "Blender must be created with a factor in order to retrieve it.");
	return factors->front();
}

std::shared_ptr<Blender> Blender::Resolve(Graphics& gfx, bool blending, std::optional<f32> factor) {
	return BindableCodex::Resolve<Blender>(gfx, blending, factor);
}

std::string Blender::GenerateUID(bool blending, std::optional<f32> factor) {
	using namespace std::string_literals;
	return typeid(Blender).name() + "#"s + (blending? "b":"n") + (factor? "#f" + std::to_string(*factor) : "");
}

std::string Blender::GetUID() const noexcept {
	return GenerateUID(blending, factors? factors->front() : std::optional<f32>{});
}
