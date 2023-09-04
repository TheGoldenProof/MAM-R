#pragma once
#include "Graphics\Drawable\Drawable.h"

class Quad : public Drawable {
public:
	struct QuadDesc {
		std::string uniqueName;
		std::string texture;
		DirectX::XMFLOAT2 size = { 1.0f, 1.0f };
		usize layer = 0;
		SIZE_MODE sizeMode = SIZE_MODE_SCALE;
		DirectX::XMFLOAT2 uvOffset = { 0, 0 };
		DirectX::XMFLOAT2 uvSize = { 1.0f, 1.0f };
		D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		std::string vertexShader = "VS_Texture.cso";
		std::string pixelShader = "PS_Texture.cso";
	};
public:
	Quad(Graphics& gfx, const QuadDesc& desc);
	Quad(Graphics& gfx, const QuadDesc& desc, std::shared_ptr<class Texture> pTex);
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	void SetRotation(f32 x, f32 y, f32 z) noexcept;
	void SetScale(f32 w, f32 h) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SpawnControlWindow(Graphics& gfx, const std::string& name) noexcept;
private:
	const std::string uniqueName;
	DirectX::XMFLOAT3 pos{0.0f, 0.0f, 0.0f};
	f32 pitch = 0;
	f32 yaw = 0;
	f32 roll = 0;
	f32 scaleW = 1.0f;
	f32 scaleH = 1.0f;
};