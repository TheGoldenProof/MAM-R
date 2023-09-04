#include "mamr_defs.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsMacros.h"
#include "Graphics/Camera.h"
#include "Graphics/Resource/DepthStencil.h"
#include "MyException.h"
#include "Windows/Window.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include <sstream>
#include <d3dcompiler.h>
#include <DirectXMath.h>

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")


Graphics::Graphics(HWND hWnd, u32 width, u32 height) : width(width), height(height), projection(), camera() {

	DXGI_SWAP_CHAIN_DESC swapDesc{};
	swapDesc.BufferDesc.Width = 0;
	swapDesc.BufferDesc.Height = 0;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapDesc.BufferDesc.RefreshRate.Denominator = 0;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = 2;
	swapDesc.OutputWindow = hWnd;
	swapDesc.Windowed = TRUE;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapDesc.Flags = 0;

	u32 swapCreateFlags = 0;
	#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif // !NDEBUG

	HRESULT hr;

	const D3D_FEATURE_LEVEL fl[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		swapCreateFlags,
		fl,
		_countof(fl),
		D3D11_SDK_VERSION,
		&swapDesc,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	));

	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget));
	
	D3D11_DEPTH_STENCIL_DESC dsd{};
	dsd.DepthEnable = TRUE;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
	wrl::ComPtr<ID3D11DepthStencilState> pDSState;
	GFX_THROW_INFO(pDevice->CreateDepthStencilState(&dsd, &pDSState));

	pContext->OMSetDepthStencilState(pDSState.Get(), 1);

	D3D11_VIEWPORT vp{};
	vp.Width = MAMR_WINW;
	vp.Height = MAMR_WINH;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1, &vp);

	ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());
}

Graphics::~Graphics() {
	ImGui_ImplDX11_Shutdown();
}

void Graphics::BeginFrame() {

	if (imguiEnabled) {
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

}

void Graphics::EndFrame() {
	if (imguiEnabled) {
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	HRESULT hr;
	#ifndef NDEBUG
	infoManager.Set();
	#endif // !NDEBUG

	if (FAILED(hr = pSwap->Present(0, 0))) {
		if (hr == DXGI_ERROR_DEVICE_REMOVED) {
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		} else throw GFX_EXCEPT(hr);
	}
}

void Graphics::BindSwapBuffer() noexcept {
	pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), nullptr);

	D3D11_VIEWPORT vp;
	vp.Width = (f32)width;
	vp.Height = (f32)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	pContext->RSSetViewports(1, &vp);
}

void Graphics::BindSwapBuffer(const DepthStencil& ds) noexcept {
	pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), ds.pDepthStencilView.Get());
}

void Graphics::ClearBuffer(f32 r, f32 g, f32 b) noexcept {
	const f32 color[]{r, g, b, 1.0f};
	pContext->ClearRenderTargetView(pTarget.Get(), color);
}

void Graphics::DrawIndexed(u32 indexCount) dbgexcept {
	GFX_THROW_INFO_ONLY(pContext->DrawIndexed(indexCount, 0, 0));
	drawCalls++;
}

void Graphics::SetProjection(DirectX::FXMMATRIX proj) noexcept { projection = proj; }
DirectX::XMMATRIX Graphics::GetProjection() const noexcept { return projection; }

void Graphics::SetCamera(DirectX::FXMMATRIX cam) noexcept { camera = cam; }

DirectX::XMMATRIX Graphics::GetCamera() const noexcept { return camera; }

void Graphics::OnResize(u32 newWidth, u32 newHeight) {
	width = newWidth;
	height = newHeight;
	
	pContext->OMSetRenderTargets(0, nullptr, nullptr);
	pTarget->Release();

	HRESULT hr;
	GFX_THROW_INFO(pSwap->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));

	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, pTarget.GetAddressOf()));
}

u32 Graphics::GetWidth() const noexcept { return width; }
u32 Graphics::GetHeight() const noexcept { return height; }

f32 Graphics::GetRefreshRate() const {
	HRESULT hr;

	wrl::ComPtr<IDXGIOutput> pDxgiOutput;
	GFX_THROW_INFO(pSwap->GetContainingOutput(&pDxgiOutput));

	DXGI_MODE_DESC empty = {};
	DXGI_MODE_DESC desc;
	wrl::ComPtr<IUnknown> pDevice_;
	pDevice.As(&pDevice_);
	GFX_THROW_INFO(pDxgiOutput->FindClosestMatchingMode(&empty, &desc, pDevice_.Get()));

	return round(static_cast<f32>(desc.RefreshRate.Numerator) / desc.RefreshRate.Denominator);
}

Graphics::HrException::HrException(i32 line, const char* file, HRESULT hr, std::vector<std::string> messages) noexcept : GfxException(line, file), hr(hr) {
	for (const auto& m : messages) {
		info += m;
		info.push_back('\n');
	}
	if (!info.empty()) info.pop_back();
}

const char* Graphics::HrException::what() const noexcept {
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (u32)GetErrorCode() << ")" << std::endl
		<< "[Error String]" << GetErrorString() << std::endl
		<< "[Description]" << GetErrorDescription() << std::endl;
	if (!info.empty()) oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
		oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept { return "Graphics Exception"; }
HRESULT Graphics::HrException::GetErrorCode() const noexcept { return hr; }
std::string Graphics::HrException::GetErrorString() const noexcept {
	return Window::Exception::TranslateErrorCode(hr);
}
std::string Graphics::HrException::GetErrorDescription() const noexcept { return std::string(); }

std::string Graphics::HrException::GetErrorInfo() const noexcept { return info; }

const char* Graphics::DeviceRemovedException::GetType() const noexcept {
	return "Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}

Graphics::InfoException::InfoException(i32 line, const char* file, std::vector<std::string> messages) noexcept : GfxException(line, file) {
	for (const auto& m : messages) {
		info += m;
		info.push_back('\n');
	}

	if (!info.empty()) info.pop_back();
}

const char* Graphics::InfoException::what() const noexcept {
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept { return "Graphics Info Exception"; }

std::string Graphics::InfoException::GetErrorInfo() const noexcept { return info; }
