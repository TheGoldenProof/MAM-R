#pragma once
#include "Graphics\Drawable\Drawable.h"
#include "Graphics\Vertex.h"

class QuadBatch : public Drawable {
private:
	template<typename T>
	class allocator : public std::allocator<T> {
	public:
		using std::allocator<T>::allocator;
		template<typename U, typename... Args>
		void construct(U*, Args&&...) {}
	};
public:
	struct BatchDesc {
		std::string uniqueName;
		usize maxQuadCount = 1024; // absolute maximum of 16383 because vertex index is stored as u16 (65535/4 = 16383)
		std::string texture;
		usize layer = 0;
		D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		std::string vertexShader = "VS_Texture.cso";
		std::string pixelShader = "PS_Texture.cso";
	};
	struct QuadBatchDesc {
		DirectX::XMFLOAT3 position = { 0, 0, 0 };
		DirectX::XMFLOAT3 rotation = { 0, 0, 0 };
		DirectX::XMFLOAT2 size = { 1.0f, 1.0f };
		SIZE_MODE sizeMode = SIZE_MODE_SCALE;
		DirectX::XMFLOAT2 uvOffset = { 0, 0 };
		DirectX::XMFLOAT2 uvSize = { 1.0f, 1.0f };
	};
protected:
	const std::string uniqueName;
	const usize maxQuadCount;
private:
	u32 texWpx, texHpx;
	bool texIsErrored;

	Vtx::VertexBuffer vbuf;
	std::vector<u16, allocator<u16>> indicies;
public:
	QuadBatch(Graphics& gfx, const BatchDesc& desc);
	QuadBatch(Graphics& gfx, const BatchDesc& desc, std::shared_ptr<class Texture> pTex);

	void Clear();
	void AddOneQuad(const QuadBatchDesc& quad);
	template<typename Iter> requires (std::is_same_v<typename std::iterator_traits<Iter>::value_type, QuadBatchDesc>)
	void AddManyQuads(Iter begin, Iter end) {
		for (Iter i = begin; i != end; i++)
			AddOneQuad(*i);
	}

	void FlushChanges(Graphics& gfx);

	DirectX::XMMATRIX GetTransformXM() const noexcept override;
};