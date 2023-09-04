#include "Graphics/Bindable/ConstantBuffers.h"

void ConstantBuffer::Update(Graphics& gfx, const CBD::Buffer& buf) {
	assert(&buf.GetRootLayoutElement() == &GetRootLayoutElement());
	INFOMAN(gfx);

	D3D11_MAPPED_SUBRESOURCE msr{};
	GFX_THROW_INFO(GetContext(gfx)->Map(pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr));
	memcpy(msr.pData, buf.GetData(), buf.GetSizeInBytes());
	GetContext(gfx)->Unmap(pConstantBuffer.Get(), 0);
}

ConstantBuffer::ConstantBuffer(Graphics& gfx, const CBD::LayoutElement& layoutRoot, u32 slot, const CBD::Buffer* pBuf) : slot(slot) {
	INFOMAN(gfx);

	D3D11_BUFFER_DESC bd{};
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	bd.ByteWidth = (u32)layoutRoot.GetSizeInBytes();
	bd.StructureByteStride = 0;
	if (pBuf != nullptr) {
		D3D11_SUBRESOURCE_DATA srd{};
		srd.pSysMem = pBuf->GetData();
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, &srd, &pConstantBuffer));
	} else {
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&bd, nullptr, &pConstantBuffer));
	}
}

void VertexConstantBuffer::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->VSSetConstantBuffers(slot, 1, pConstantBuffer.GetAddressOf());
}

std::string VertexConstantBuffer::GenerateUID(u32 slot) {
	using namespace std::string_literals;
	return typeid(VertexConstantBuffer).name() + "#"s + std::to_string(slot);
}

std::string PixelConstantBuffer::GenerateUID(u32 slot) {
	using namespace std::string_literals;
	return typeid(VertexConstantBuffer).name() + "#"s + std::to_string(slot);
}

void PixelConstantBuffer::Bind(Graphics& gfx) noexcept {
	GetContext(gfx)->PSSetConstantBuffers(slot, 1, pConstantBuffer.GetAddressOf());
}
