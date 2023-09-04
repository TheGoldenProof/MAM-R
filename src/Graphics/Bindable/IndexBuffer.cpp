#include "Graphics/Bindable/IndexBuffer.h"
#include "Graphics/GraphicsMacros.h"
#include "Graphics/Bindable/BindableCodex.h"

IndexBuffer::IndexBuffer(Graphics& gfx, const std::string& tag, const u16* pData, usize count, bool updatable_, u32 maxCount_)
	: tag(tag), indexCount(u32(count)), updatable(updatable_), maxCount(u32(max(maxCount_, count))) {
	INFOMAN(gfx);

	D3D11_BUFFER_DESC bd{};
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.Usage = updatable? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = updatable? D3D11_CPU_ACCESS_WRITE : 0;
	bd.MiscFlags = 0;
	bd.ByteWidth = maxCount * sizeof(u16);
	bd.StructureByteStride = sizeof(u16);
	D3D11_SUBRESOURCE_DATA srd{};
	srd.pSysMem = pData;
	GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, &srd, &pIndexBuffer));
}

void IndexBuffer::Update(Graphics& gfx, const u16* data, usize count_) {
	if (!updatable) return;
	INFOMAN(gfx);

	D3D11_MAPPED_SUBRESOURCE msr{};
	GFX_THROW_INFO(GetContext(gfx)->Map(pIndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	const usize dataSize = maxCount * sizeof(u16);
	const usize vbufSize = count_ * sizeof(u16);
	memcpy_s(msr.pData, dataSize, data, min(dataSize, vbufSize));
	GetContext(gfx)->Unmap(pIndexBuffer.Get(), 0);

	indexCount = u32(min(count_, maxCount));
}

void IndexBuffer::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
}

u32 IndexBuffer::GetCount() const noexcept { return indexCount; }

std::shared_ptr<IndexBuffer> IndexBuffer::Resolve(Graphics& gfx, const std::string& tag, const u16* pData, usize count, bool updatable_, u32 maxCount_) {
	assert(tag != "?");
	return BindableCodex::Resolve<IndexBuffer>(gfx, tag, pData, count, updatable_, maxCount_);
}
std::string IndexBuffer::GenerateUID_(const std::string& tag, bool updatable_, u32 maxCount_) {
	using namespace std::string_literals;
	return typeid(IndexBuffer).name() + "#"s + tag + ","s + (updatable_? "d"s : "s"s) + ","s + std::to_string(maxCount_);
}
std::string IndexBuffer::GetUID() const noexcept {
	return GenerateUID_(tag, updatable, maxCount);
}