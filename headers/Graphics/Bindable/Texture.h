#pragma once
#include "Graphics/Bindable/Bindable.h"
#include "Graphics/Resource/RenderTarget.h"
#include "Graphics/Surface.h"

class Texture : public Bindable {
public:
	Texture(Graphics& gfx, const std::wstring& path, u32 slot = 0);

	static void UnbindTexture(Graphics& gfx, u32 slot) noexcept;

	virtual u32 GetWidth() const noexcept = 0;
	virtual u32 GetHeight() const noexcept = 0;

	virtual bool IsErrored() const noexcept { return false; }
protected:
	std::wstring path;
	u32 slot;
};

class SurfaceTexture : public Texture {
public:
	SurfaceTexture(Graphics& gfx, const std::wstring& path, u32 slot = 0);
	SurfaceTexture(Graphics& gfx, Surface&& s, const std::wstring& name, u32 slot = 0);

	Surface& GetSurface() noexcept;
	virtual u32 GetWidth() const noexcept override;
	virtual u32 GetHeight() const noexcept override;
	virtual bool IsErrored() const noexcept override;

	void Bind(Graphics& gfx) noexcept override;
	static std::shared_ptr<SurfaceTexture> Resolve(Graphics& gfx, const std::wstring& path, u32 slot = 0);
	static std::string GenerateUID(const std::wstring& path, u32 slot = 0);
	std::string GetUID() const noexcept override;
private:
	Surface surface;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
};

class RenderTargetTexture : public Texture, public RenderTarget {
public:
	RenderTargetTexture(Graphics& gfx, u32 width, u32 height, const std::wstring& name, u32 slot = 0);
	RenderTargetTexture(Graphics& gfx, const std::wstring& name, u32 slot = 0);
	
	RenderTarget& GetRenderTarget() noexcept;
	virtual u32 GetWidth() const noexcept override;
	virtual u32 GetHeight() const noexcept override;

	void Bind(Graphics& gfx) noexcept override;
	static std::shared_ptr<RenderTargetTexture> Resolve(Graphics& gfx, u32 width, u32 height, const std::wstring& name, u32 slot = 0);
	static std::shared_ptr<RenderTargetTexture> Resolve(Graphics& gfx, const std::wstring& name, u32 slot = 0);
	static std::string GenerateUID(u32 width, u32 height, const std::wstring& name, u32 slot = 0);
	static std::string GenerateUID(const std::wstring& name, u32 slot = 0);
	std::string GetUID() const noexcept override;
private:

};