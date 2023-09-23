#pragma once
#include "Graphics/Bindable/Bindable.h"

class IndexBuffer : public Bindable {
public:
	IndexBuffer(Graphics& gfx, const std::string& tag, const u32* pData, usize count, bool updatable = false, u32 maxCount = 0);

	void Update(Graphics& gfx, const u32* data, usize count);

	void Bind(Graphics& gfx) noexcept override;
	u32 GetCount() const noexcept;

	static std::shared_ptr<IndexBuffer> Resolve(Graphics& gfx, const std::string& tag, const u32* pData, usize count, bool updatable = false, u32 maxCount = 0);
	static std::string GenerateUID(const std::string& tag, const u32* pData, usize count, bool updatable = false, u32 maxCount = 0) {
		UNREFERENCED_PARAMETER(pData);
		UNREFERENCED_PARAMETER(count);
		return GenerateUID_(tag, updatable, maxCount);
	}
	std::string GetUID() const noexcept override;
private:
	static std::string GenerateUID_(const std::string& tag, bool updatable = false, u32 maxCount = 0);
protected:
	std::string tag;
	u32 indexCount;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
	bool updatable;
	u32 maxCount;
};