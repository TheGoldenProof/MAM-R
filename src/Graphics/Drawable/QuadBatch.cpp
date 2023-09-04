#include "Graphics/Bindable/Bindables.h"
#include "Graphics\Drawable\QuadBatch.h"

QuadBatch::QuadBatch(Graphics& gfx, const BatchDesc& desc) 
	: QuadBatch(gfx, desc, std::move(desc.texture.front() == '@' ?
		static_cast<std::shared_ptr<Texture>>(RenderTargetTexture::Resolve(gfx, desc.texture, 0))
		: static_cast<std::shared_ptr<Texture>>(SurfaceTexture::Resolve(gfx, desc.texture, 0)))) {}

QuadBatch::QuadBatch(Graphics& gfx, const BatchDesc& desc, std::shared_ptr<class Texture> pTex)
	: uniqueName(desc.uniqueName),
	  maxQuadCount(desc.maxQuadCount),
	  texIsErrored(pTex->IsErrored()),
	  texWpx(pTex->GetWidth()), texHpx(pTex->GetHeight()) {
	using Type = Vtx::VertexLayout::ElementType;

	Vtx::VertexLayout l;
	l.Append(Vtx::VertexLayout::Position3D);
	l.Append(Vtx::VertexLayout::Texture2D);
	vbuf = Vtx::VertexBuffer(l);
	vbuf.Reserve(maxQuadCount*4);

	indicies.resize(maxQuadCount*6);
	for (u16 i = 0; i < maxQuadCount; i++) {
		indicies[i*6+0] = 0 + i*4;
		indicies[i*6+1] = 1 + i*4;
		indicies[i*6+2] = 2 + i*4;
		indicies[i*6+3] = 1 + i*4;
		indicies[i*6+4] = 3 + i*4;
		indicies[i*6+5] = 2 + i*4;
	}
	indicies.resize(0);

	const std::string tag = "$quadbatch." + std::to_string(maxQuadCount);
	pVerticies = VertexBuffer::Resolve(gfx, tag + "." + uniqueName, vbuf, true, u32(maxQuadCount) * 4);
	pIndicies = IndexBuffer::Resolve(gfx, tag, indicies.data(), indicies.size(), true, u32(maxQuadCount) * 6);
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Technique main("Main");
	QueueStep only(desc.layer);
	only.AddBindable(std::move(pTex));
	only.AddBindable(Sampler::Resolve(gfx, desc.filter));

	auto pVs = VertexShader::Resolve(gfx, desc.vertexShader);
	auto pVsbc = pVs->GetBytecode();
	only.AddBindable(std::move(pVs));

	only.AddBindable(PixelShader::Resolve(gfx, desc.pixelShader));

	only.AddBindable(InputLayout::Resolve(gfx, l, pVsbc));
	only.AddBindable(std::make_shared<TransformCBuf>(gfx));

	main.AddStep(std::move(only));
	AddTechnique(std::move(main));
}

void QuadBatch::Clear() {
	vbuf.Clear();
	indicies.resize(0);
}

void QuadBatch::AddOneQuad(const QuadBatchDesc& quad) {
	using namespace DirectX;
	XMVECTOR pos = XMLoadFloat3(&quad.position);
	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(quad.rotation.x, quad.rotation.y, quad.rotation.z);
	XMVECTOR scl = XMLoadFloat2(&quad.size);
	if (quad.sizeMode == SIZE_MODE_SCALE) {
		scl = XMVectorMultiply(scl, XMVectorMultiply(XMVectorSet(f32(texWpx), f32(texHpx), 0, 0), XMLoadFloat2(&quad.uvSize)));
	}
	XMMATRIX trfm = XMMatrixAffineTransformation(scl, XMVectorZero(), rot, pos);
	XMVECTOR vverts[4] = {  { -0.5f,  0.5f, 0, 0 },
							{  0.5f,  0.5f, 0, 0 },
							{ -0.5f, -0.5f, 0, 0 },
							{  0.5f, -0.5f, 0, 0 }, };

	for (XMVECTOR& vec : vverts) 
		vec = XMVector3Transform(vec, trfm);

	XMFLOAT3 fverts[4];
	for (usize i = 0; i < 4; i++)
		XMStoreFloat3(fverts + i, vverts[i]);

	vbuf.EmplaceBack(fverts[0], XMFLOAT2{ quad.uvOffset.x, quad.uvOffset.y });
	vbuf.EmplaceBack(fverts[1], XMFLOAT2{ quad.uvOffset.x + quad.uvSize.x, quad.uvOffset.y });
	vbuf.EmplaceBack(fverts[2], XMFLOAT2{ quad.uvOffset.x, quad.uvOffset.y + quad.uvSize.y });
	vbuf.EmplaceBack(fverts[3], XMFLOAT2{ quad.uvOffset.x + quad.uvSize.x, quad.uvOffset.y + quad.uvSize.y });

	indicies.resize(indicies.size() + 6);
}

void QuadBatch::FlushChanges(Graphics& gfx) {
	pVerticies->Update(gfx, vbuf);
	pIndicies->Update(gfx, indicies.data(), indicies.size());
}

DirectX::XMMATRIX QuadBatch::GetTransformXM() const noexcept {
	return DirectX::XMMatrixIdentity();
}

