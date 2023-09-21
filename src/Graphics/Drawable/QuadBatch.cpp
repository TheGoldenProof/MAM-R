#include "Graphics/Bindable/Bindables.h"
#include "Graphics\Drawable\QuadBatch.h"

QuadBatch::QuadBatch(const std::string& name, usize maxQuadCount) : uniqueName(name), maxQuadCount(maxQuadCount) {}

void QuadBatch::Clear() {
	vbuf.Clear();
	indicies.resize(0);
}

void QuadBatch::FlushChanges(Graphics& gfx) {
	pVerticies->Update(gfx, vbuf);
	pIndicies->Update(gfx, indicies.data(), indicies.size());
}

DirectX::XMFLOAT3 QuadBatch::GetPos() const noexcept {
	return pos;
}

void QuadBatch::SetPos(DirectX::XMFLOAT3 pos_) noexcept {
	pos = pos_;
}

DirectX::XMFLOAT3 QuadBatch::GetRotation() const noexcept {
	return { pitch, yaw, roll };
}

void QuadBatch::SetRotation(f32 pitch_, f32 yaw_, f32 roll_) noexcept {
	pitch = pitch_;
	yaw = yaw_;
	roll = roll_;
}

DirectX::XMFLOAT2 QuadBatch::GetScale() const noexcept {
	return { scaleW, scaleH };
}

void QuadBatch::SetScale(f32 w, f32 h) noexcept {
	scaleW = w;
	scaleH = h;
}

DirectX::XMMATRIX QuadBatch::GetTransformXM() const noexcept {
	DirectX::XMVECTOR pos_ = DirectX::XMVectorSet(pos.x, pos.y, pos.z, 0.0f);
	DirectX::XMVECTOR rot_ = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
	DirectX::XMVECTOR scl_ = DirectX::XMVectorSet(scaleW, scaleH, 1.0f, 1.0f);
	return DirectX::XMMatrixAffineTransformation(scl_, DirectX::XMVectorZero(), rot_, pos_);
}

QuadBatchTextured::QuadBatchTextured(Graphics& gfx, const BatchDesc& desc)
	: QuadBatchTextured(gfx, desc, std::move(desc.texture.front() == '@' ?
		static_cast<std::shared_ptr<Texture>>(RenderTargetTexture::Resolve(gfx, desc.texture, 0))
		: static_cast<std::shared_ptr<Texture>>(SurfaceTexture::Resolve(gfx, desc.texture, 0)))) {
}

QuadBatchTextured::QuadBatchTextured(Graphics& gfx, const BatchDesc& desc, std::shared_ptr<class Texture> pTex)
	: QuadBatch(desc.uniqueName, desc.maxQuadCount),
	texIsErrored(pTex->IsErrored()),
	texWpx(pTex->GetWidth()), texHpx(pTex->GetHeight()) {
	using Type = Vtx::VertexLayout::ElementType;

	Vtx::VertexLayout l;
	l.Append(Vtx::VertexLayout::Position3D);
	l.Append(Vtx::VertexLayout::Texture2D);
	vbuf = Vtx::VertexBuffer(l);
	vbuf.Reserve(maxQuadCount * 4);

	indicies.resize(maxQuadCount * 6);
	for (u16 i = 0; i < maxQuadCount; i++) {
		indicies[i * 6 + 0] = 0 + i * 4;
		indicies[i * 6 + 1] = 1 + i * 4;
		indicies[i * 6 + 2] = 2 + i * 4;
		indicies[i * 6 + 3] = 1 + i * 4;
		indicies[i * 6 + 4] = 3 + i * 4;
		indicies[i * 6 + 5] = 2 + i * 4;
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

void QuadBatchTextured::AddOneQuad(const QuadDesc& quad) {
	using namespace DirectX;
	XMVECTOR pos_ = XMLoadFloat3(&quad.position);
	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(quad.rotation.x, quad.rotation.y, quad.rotation.z);
	XMVECTOR scl = XMLoadFloat2(&quad.size);
	if (quad.sizeMode == SIZE_MODE_SCALE) {
		scl = XMVectorMultiply(scl, XMVectorMultiply(XMVectorSet(f32(texWpx), f32(texHpx), 0, 0), XMLoadFloat2(&quad.uvSize)));
	}
	XMMATRIX trfm = XMMatrixAffineTransformation(scl, XMVectorZero(), rot, pos_);
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

QuadBatchColored::QuadBatchColored(Graphics& gfx, const BatchDesc& desc)
	: QuadBatch(desc.uniqueName, desc.maxQuadCount) {
	using Type = Vtx::VertexLayout::ElementType;

	Vtx::VertexLayout l;
	l.Append(Vtx::VertexLayout::Position3D);
	l.Append(Vtx::VertexLayout::Float4Color);
	vbuf = Vtx::VertexBuffer(l);
	vbuf.Reserve(maxQuadCount * 4);

	indicies.resize(maxQuadCount * 6);
	for (u16 i = 0; i < maxQuadCount; i++) {
		indicies[i * 6 + 0] = 0 + i * 4;
		indicies[i * 6 + 1] = 1 + i * 4;
		indicies[i * 6 + 2] = 2 + i * 4;
		indicies[i * 6 + 3] = 1 + i * 4;
		indicies[i * 6 + 4] = 3 + i * 4;
		indicies[i * 6 + 5] = 2 + i * 4;
	}
	indicies.resize(0);

	const std::string tag = "$quadbatch." + std::to_string(maxQuadCount);
	pVerticies = VertexBuffer::Resolve(gfx, tag + "." + uniqueName, vbuf, true, u32(maxQuadCount) * 4);
	pIndicies = IndexBuffer::Resolve(gfx, tag, indicies.data(), indicies.size(), true, u32(maxQuadCount) * 6);
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Technique main("Main");
	QueueStep only(desc.layer);

	auto pVs = VertexShader::Resolve(gfx, desc.vertexShader);
	auto pVsbc = pVs->GetBytecode();
	only.AddBindable(std::move(pVs));

	only.AddBindable(PixelShader::Resolve(gfx, desc.pixelShader));

	only.AddBindable(InputLayout::Resolve(gfx, l, pVsbc));
	only.AddBindable(std::make_shared<TransformCBuf>(gfx));

	main.AddStep(std::move(only));
	AddTechnique(std::move(main));
}

void QuadBatchColored::AddOneQuad(const QuadDesc& quad) {
	using namespace DirectX;
	XMVECTOR pos_ = XMLoadFloat3(&quad.position);
	XMVECTOR rot = XMQuaternionRotationRollPitchYaw(quad.rotation.x, quad.rotation.y, quad.rotation.z);
	XMVECTOR scl = XMLoadFloat2(&quad.size);
	XMMATRIX trfm = XMMatrixAffineTransformation(scl, XMVectorZero(), rot, pos_);
	XMVECTOR vverts[4] = { { -0.5f,  0.5f, 0, 0 },
							{  0.5f,  0.5f, 0, 0 },
							{ -0.5f, -0.5f, 0, 0 },
							{  0.5f, -0.5f, 0, 0 }, };

	for (XMVECTOR& vec : vverts)
		vec = XMVector3Transform(vec, trfm);

	XMFLOAT3 fverts[4];
	for (usize i = 0; i < 4; i++)
		XMStoreFloat3(fverts + i, vverts[i]);

	for (u32 i = 0; i < 4; i++) {
		auto color = quad.colors[quad.singleColor ? 0 : i];
		vbuf.EmplaceBack(fverts[i], DirectX::XMFLOAT4{ color[0], color[1], color[2], color[3] });
	}

	indicies.resize(indicies.size() + 6);
}
