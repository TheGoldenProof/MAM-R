#include "Graphics/DxgiInfoManager.h"
#include "Windows/Window.h"
#include "Windows/WindowMacros.h"
#include "Graphics/Graphics.h"
#include <memory>

#pragma comment(lib, "dxguid.lib")

#define GFX_THROW_NOINFO(hrcall) if(FAILED(hr = (hrcall))) throw Graphics::HrException(__LINE__, __FILE__, (hr))

DxgiInfoManager::DxgiInfoManager() {
	typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

	const auto hModDxgiDebug = LoadLibraryEx(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	if (!hModDxgiDebug) throw MYWND_LAST_EXCEPT();

	const auto DxgiGetDebugInterface = (DXGIGetDebugInterface)((void*)GetProcAddress(hModDxgiDebug, "DXGIGetDebugInterface"));
	if (!DxgiGetDebugInterface) throw MYWND_LAST_EXCEPT();

	HRESULT hr;
	GFX_THROW_NOINFO(DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &pDxgiInfoQueue));
}

void DxgiInfoManager::Set() noexcept {
	next = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
}

std::vector<std::string> DxgiInfoManager::GetMessages() const {
	std::vector<std::string> messages;
	const u64 end = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
	for (auto i = next; i < end; i++) {
		HRESULT hr;
		usize messageLength = 0;

		GFX_THROW_NOINFO(pDxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength));

		auto bytes = std::make_unique<byte[]>(messageLength);
		auto pMessage = (DXGI_INFO_QUEUE_MESSAGE*)(bytes.get());

		GFX_THROW_NOINFO(pDxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, pMessage, &messageLength));
		messages.emplace_back(pMessage->pDescription);
	}

	return messages;
}
