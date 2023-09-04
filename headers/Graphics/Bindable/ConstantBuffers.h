#pragma once
#include "Graphics/Bindable/Bindable.h"
#include "Graphics/ConstantBufferDyn.h"
#include "Graphics/GraphicsMacros.h"
#include <concepts>

class ConstantBuffer : public Bindable {
public:
	void Update(Graphics& gfx, const CBD::Buffer& buf);

	virtual const CBD::LayoutElement& GetRootLayoutElement() const noexcept = 0;
protected:
	ConstantBuffer(Graphics& gfx, const CBD::LayoutElement& layoutRoot, u32 slot, const CBD::Buffer* pBuf);
protected:
	u32 slot;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
};

template<class T> requires std::derived_from<T, ConstantBuffer>
class ConstantBufferCaching : public T {
public:
	ConstantBufferCaching(Graphics& gfx, const CBD::CookedLayout& layout, u32 slot)
		: T(gfx, *layout.ShareRoot(), slot, nullptr), buf(layout) {}
	ConstantBufferCaching(Graphics& gfx, const CBD::Buffer& buf, u32 slot)
		: T(gfx, buf.GetRootLayoutElement(), slot, &buf), buf(buf) {}

	virtual const CBD::LayoutElement& GetRootLayoutElement() const noexcept override { return buf.GetRootLayoutElement(); }
	const CBD::Buffer& GetBuffer() const noexcept { return buf; }
	CBD::Buffer& GetBuffer() noexcept { dirty = true; return buf; }
	void SetBuffer(const CBD::Buffer& buf_) { buf.CopyFrom(buf_); dirty = true; }
	void Update(Graphics& gfx) { T::Update(gfx, buf); }
	void Bind(Graphics& gfx) noexcept override {
		if (dirty) {
			T::Update(gfx, buf);
			dirty = false;
		}
		T::Bind(gfx);
	}
	void Accept(TechniqueProbe& probe) override {
		if (probe.VisitBuffer(buf)) dirty = true;
	}
private:
	bool dirty = false;
	CBD::Buffer buf;
};

class VertexConstantBuffer : public ConstantBuffer {
public:
	using ConstantBuffer::ConstantBuffer;
	void Bind(Graphics& gfx) noexcept override;
	static std::string GenerateUID(const CBD::LayoutElement& layoutRoot, u32 slot, const CBD::Buffer* pBuf) {
		UNREFERENCED_PARAMETER(layoutRoot);
		UNREFERENCED_PARAMETER(pBuf);
		return GenerateUID(slot);
	}
	static std::string GenerateUID(u32 slot = 0);
	virtual std::string GetUID() const noexcept override { return GenerateUID(slot); }
};

class PixelConstantBuffer : public ConstantBuffer {
public:
	using ConstantBuffer::ConstantBuffer;
	void Bind(Graphics& gfx) noexcept override;
	static std::string GenerateUID(const CBD::LayoutElement& layoutRoot, u32 slot, const CBD::Buffer* pBuf) {
		UNREFERENCED_PARAMETER(layoutRoot);
		UNREFERENCED_PARAMETER(pBuf);
		return GenerateUID(slot);
	}
	static std::string GenerateUID(u32 slot = 0);
	virtual std::string GetUID() const noexcept override { return GenerateUID(slot); }
};

using VertexConstantBufferCaching = ConstantBufferCaching<VertexConstantBuffer>;
using PixelConstantBufferCaching = ConstantBufferCaching<PixelConstantBuffer>;
