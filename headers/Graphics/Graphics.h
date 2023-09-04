#pragma once

#include "Graphics/DxgiInfoManager.h"
#include "MyException.h"
#include "TGLib/TGLib.h"
#include "Windows/framework.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#pragma warning(disable:4265)
#include <wrl.h>
#pragma warning(default:4265)
#include <vector>

class DepthStencil;

class Graphics {
	friend class GraphicsResource;
public:
	class GfxException : public MyException {
		using MyException::MyException;
	};
	class HrException : public GfxException {
	public:
		HrException(i32 line, const char* file, HRESULT hr, std::vector<std::string> messages = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	class InfoException : public GfxException {
	public:
		InfoException(i32 line, const char* file, std::vector<std::string> messages) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept;
	private:
		std::string info;
	};
	class DeviceRemovedException : public HrException {
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};
public:
	u32 drawCalls = 0;

	Graphics(HWND hWnd, u32 width, u32 height);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();

	void BeginFrame();
	void EndFrame();

	void BindSwapBuffer() noexcept;
	void BindSwapBuffer(const DepthStencil& ds) noexcept;

	void ClearBuffer(f32 r, f32 g, f32 b) noexcept;
	void DrawIndexed(u32 indexCount) dbgexcept;

	void SetProjection(DirectX::FXMMATRIX proj) noexcept;
	DirectX::XMMATRIX GetProjection() const noexcept;
	void SetCamera(DirectX::FXMMATRIX cam) noexcept;
	DirectX::XMMATRIX GetCamera() const noexcept;

	void OnResize(u32 newWidth, u32 newHeight);
	u32 GetWidth() const noexcept;
	u32 GetHeight() const noexcept;
	f32 GetRefreshRate() const;

	void EnableImGui() noexcept { imguiEnabled = true; }
	void DisableImGui() noexcept { imguiEnabled = false; }
	bool IsImguiEnabled() const noexcept { return imguiEnabled; }

private:
	u32 width, height;

	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX camera;

	#ifndef NDEBUG
	mutable DxgiInfoManager infoManager;
	#endif // !NDEBUG
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;

	bool imguiEnabled = true;
};