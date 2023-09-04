#pragma once
#include "TGLib/TGLib.h"
#include "Windows/framework.h"
#include <dxgidebug.h>
#include <string>
#include <vector>
#pragma warning(disable:4265)
#include <wrl.h>
#pragma warning(default:4265)

class DxgiInfoManager {
public:
	DxgiInfoManager();
	DxgiInfoManager(const DxgiInfoManager&) = delete;
	DxgiInfoManager& operator=(const DxgiInfoManager&) = delete;
	~DxgiInfoManager() = default;

	void Set() noexcept;
	std::vector<std::string> GetMessages() const;
private:
	u64 next = 0;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> pDxgiInfoQueue;
};