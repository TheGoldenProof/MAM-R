#pragma once
#include "Graphics/Bindable/Bindable.h"
#include "Graphics/GraphicsMacros.h"
#include "Graphics/Vertex.h"

class VertexBuffer : public Bindable {
public:
	VertexBuffer(Graphics& gfx, const std::string& tag, const Vtx::VertexBuffer& vbuf, bool updatable = false, u32 maxCount = 0);
	VertexBuffer(Graphics& gfx, const Vtx::VertexBuffer& vbuf, bool updatable = false, u32 maxCount = 0);

	void Update(Graphics& gfx, const Vtx::VertexBuffer& vbuf);

	void Bind(Graphics& gfx) noexcept override;
	const Vtx::VertexLayout& GetLayout() const noexcept;
	static std::shared_ptr<VertexBuffer> Resolve(Graphics& gfx, const std::string& tag, const Vtx::VertexBuffer& vbuf, bool updatable = false, u32 maxCount = 0);
	static std::string GenerateUID(const std::string& tag, const Vtx::VertexBuffer& vbuf, bool updatable = false, u32 maxCount = 0) {
		UNREFERENCED_PARAMETER(vbuf);
		return GenerateUID_(tag, updatable, maxCount);
	}
	std::string GetUID() const noexcept override;
private:
	static std::string GenerateUID_(const std::string& tag, bool updatable = false, u32 maxCount = 0);
protected:
	std::string tag;
	u32 stride;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
	Vtx::VertexLayout layout;
	bool updatable;
	u32 maxCount;
};