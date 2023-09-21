#pragma once
#include "Graphics\Drawable\Drawable.h"
#include <array>

class Quad : public Drawable {
public:
	DirectX::XMFLOAT3 GetPos() const noexcept;
	void SetPos(f32 x, f32 y, f32 z) noexcept { SetPos({ x, y, z }); }
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	DirectX::XMFLOAT3 GetRotation() const noexcept;
	void SetRotation(f32 x, f32 y, f32 z) noexcept;
	void SetRotation(DirectX::XMFLOAT3 rot) noexcept { SetRotation(rot.x, rot.y, rot.z); }
	DirectX::XMFLOAT2 GetScale() const noexcept;
	void SetScale(f32 w, f32 h) noexcept;
	void SetScale(DirectX::XMFLOAT2 scl) noexcept { SetScale(scl.x, scl.y); }

	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SpawnControlWindow(Graphics& gfx, const std::string& name) noexcept;
protected:
	Quad(const std::string& name);
protected:
	const std::string uniqueName;
private:
	DirectX::XMFLOAT3 pos{0.0f, 0.0f, 0.0f};
	f32 pitch = 0;
	f32 yaw = 0;
	f32 roll = 0;
	f32 scaleW = 1.0f;
	f32 scaleH = 1.0f;
};

class QuadTextured : public Quad {
public:
	struct QuadDesc {
		std::string uniqueName;
		DirectX::XMFLOAT2 size = { 1.0f, 1.0f };
		usize layer = 0;
		std::string vertexShader = "VS_Texture.cso";
		std::string pixelShader = "PS_Texture.cso";
		std::string texture;
		SIZE_MODE sizeMode = SIZE_MODE_SCALE;
		DirectX::XMFLOAT2 uvOffset = { 0, 0 };
		DirectX::XMFLOAT2 uvSize = { 1.0f, 1.0f };
		D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	};
public:
	QuadTextured(Graphics& gfx, const QuadDesc& desc);
	QuadTextured(Graphics& gfx, const QuadDesc& desc, std::shared_ptr<class Texture> pTex);
};

class QuadColored : public Quad {
public:
	struct QuadDesc {
		using colorArr_t = std::array<std::array<f32, 4>, 4>;
		std::string uniqueName;
		DirectX::XMFLOAT2 size = { 1.0f, 1.0f };
		usize layer = 0;
		std::string vertexShader = "Colored_VS.cso";
		std::string pixelShader = "Colored_PS.cso";
		bool singleColor = true;
		colorArr_t colors = { {{0.5f, 0.5f, 0.5f, 0.5f}, {}, {}, {}} };
	};
public:
	QuadColored(Graphics& gfx, const QuadDesc& desc);
};