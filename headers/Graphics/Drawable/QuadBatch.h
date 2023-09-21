#pragma once
#include "Graphics\Drawable\Drawable.h"
#include "Graphics\Vertex.h"
#include <array>

class QuadBatch : public Drawable {
protected:
	template<typename T>
	class allocator : public std::allocator<T> {
	public:
		using std::allocator<T>::allocator;
		template<typename U, typename... Args>
		void construct(U*, Args&&...) {}
	};
public:
protected:
	const std::string uniqueName;
	const usize maxQuadCount;

	Vtx::VertexBuffer vbuf;
	std::vector<u16, allocator<u16>> indicies;
private:
	DirectX::XMFLOAT3 pos{ 0.0f, 0.0f, 0.0f };
	f32 pitch = 0;
	f32 yaw = 0;
	f32 roll = 0;
	f32 scaleW = 1.0f;
	f32 scaleH = 1.0f;
public:
	QuadBatch(const std::string& name, usize maxQuadCount);

	void Clear();
	void FlushChanges(Graphics& gfx);

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
};

class QuadBatchTextured : public QuadBatch {
	struct BatchDesc {
		std::string uniqueName;
		usize maxQuadCount = 1024; // absolute maximum of 16383 because vertex index is stored as u16 (65535/4 = 16383)
		std::string texture;
		usize layer = 0;
		D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		std::string vertexShader = "VS_Texture.cso";
		std::string pixelShader = "PS_Texture.cso";
	};
	struct QuadDesc {
		DirectX::XMFLOAT3 position = { 0, 0, 0 };
		DirectX::XMFLOAT3 rotation = { 0, 0, 0 };
		DirectX::XMFLOAT2 size = { 1.0f, 1.0f };
		SIZE_MODE sizeMode = SIZE_MODE_SCALE;
		DirectX::XMFLOAT2 uvOffset = { 0, 0 };
		DirectX::XMFLOAT2 uvSize = { 1.0f, 1.0f };
	};
private:
	u32 texWpx, texHpx;
	bool texIsErrored;
public:
	QuadBatchTextured(Graphics& gfx, const BatchDesc& desc);
	QuadBatchTextured(Graphics& gfx, const BatchDesc& desc, std::shared_ptr<class Texture> pTex);

	void AddOneQuad(const QuadDesc& quad);
	template<typename Iter> requires (std::is_same_v<typename std::iterator_traits<Iter>::value_type, QuadDesc>)
		void AddManyQuads(Iter begin, Iter end) {
		for (Iter i = begin; i != end; i++)
			AddOneQuad(*i);
	}
};

class QuadBatchColored : public QuadBatch {
public:
	struct BatchDesc {
		std::string uniqueName;
		usize maxQuadCount = 1024; // absolute maximum of 16383 because vertex index is stored as u16 (65535/4 = 16383)
		usize layer = 0;
		std::string vertexShader = "Colored_VS.cso";
		std::string pixelShader = "Colored_PS.cso";
	};
	struct QuadDesc {
		using colorArr_t = std::array<std::array<f32, 4>, 4>;
		DirectX::XMFLOAT3 position = { 0, 0, 0 };
		DirectX::XMFLOAT3 rotation = { 0, 0, 0 };
		DirectX::XMFLOAT2 size = { 1.0f, 1.0f };
		bool singleColor = true;
		colorArr_t colors = { { {0.5f, 0.5f, 0.5f, 0.5f}, {}, {}, {} } };
	};
public:
	QuadBatchColored(Graphics& gfx, const BatchDesc& desc);

	void AddOneQuad(const QuadDesc& quad);
	template<typename Iter> requires (std::is_same_v<typename std::iterator_traits<Iter>::value_type, QuadDesc>)
		void AddManyQuads(Iter begin, Iter end) {
		for (Iter i = begin; i != end; i++)
			AddOneQuad(*i);
	}
};