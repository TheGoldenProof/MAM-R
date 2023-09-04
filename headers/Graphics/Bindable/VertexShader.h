#pragma once
#include "Graphics/Bindable/Bindable.h"

class VertexShader : public Bindable {
public:
	VertexShader(Graphics& gfx, std::string const& path);

	ID3DBlob* GetBytecode() const noexcept;

	void Bind(Graphics& gfx) noexcept override;
	static std::shared_ptr<VertexShader> Resolve(Graphics& gfx, const std::string& path);
	static std::string GenerateUID(const std::string& path);
	std::string GetUID() const noexcept override;

protected:
	std::string path;
	Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
};