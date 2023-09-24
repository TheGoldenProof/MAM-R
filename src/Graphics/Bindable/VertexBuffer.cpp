#include "Graphics/Bindable/VertexBuffer.h"
#include "Graphics/Bindable/BindableCodex.h"

VertexBuffer::VertexBuffer(Graphics& gfx, const Vtx::VertexBuffer& vbuf, bool updatable_, u32 maxCount_) 
	: VertexBuffer(gfx, "?", vbuf, updatable_, maxCount_) {}

VertexBuffer::VertexBuffer(Graphics& gfx, const std::string& tag, const Vtx::VertexBuffer& vbuf, bool updatable_, u32 maxCount_)
	: stride((u32)vbuf.GetLayout().Size()), tag(tag), layout(vbuf.GetLayout()), updatable(updatable_), maxCount(std::max(maxCount_, (u32)vbuf.Size())) {
	INFOMAN(gfx);

	D3D11_BUFFER_DESC bd{};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = updatable ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = updatable ? D3D11_CPU_ACCESS_WRITE : 0;
	bd.MiscFlags = 0;
	bd.ByteWidth = stride * maxCount;
	bd.StructureByteStride = stride;
	D3D11_SUBRESOURCE_DATA srd{};
	srd.pSysMem = vbuf.GetData();
	GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, &srd, &pVertexBuffer));
}

void VertexBuffer::Update(Graphics& gfx, const Vtx::VertexBuffer& vbuf) {
	if (!updatable) return;
	assert(vbuf.GetLayout() == layout);
	INFOMAN(gfx);

	D3D11_MAPPED_SUBRESOURCE msr{};
	GFX_THROW_INFO(GetContext(gfx)->Map(pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	const usize dataSize = maxCount * stride;
	const usize vbufSize = vbuf.SizeBytes();
	memcpy_s(msr.pData, dataSize, vbuf.GetData(), std::min(dataSize, vbufSize));
	GetContext(gfx)->Unmap(pVertexBuffer.Get(), 0);
}

void VertexBuffer::Bind(Graphics& gfx) noexcept {
	const u32 offset = 0;
	GetContext(gfx)->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);
}

const Vtx::VertexLayout& VertexBuffer::GetLayout() const noexcept { return layout; }

std::shared_ptr<VertexBuffer> VertexBuffer::Resolve(Graphics& gfx, const std::string& tag, const Vtx::VertexBuffer& vbuf, bool updatable_, u32 maxCount_) {
	assert(tag != "?");
	return BindableCodex::Resolve<VertexBuffer>(gfx, tag, vbuf, updatable_, maxCount_);
}

std::string VertexBuffer::GenerateUID_(const std::string& tag, bool updatable_, u32 maxCount_) {
	using namespace std::string_literals;
	return typeid(VertexBuffer).name() + "#"s + tag + ","s + (updatable_ ? "d"s : "s"s) + ","s + std::to_string(maxCount_);
}

std::string VertexBuffer::GetUID() const noexcept {
	return GenerateUID_(tag, updatable, maxCount);
}