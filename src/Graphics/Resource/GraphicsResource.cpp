#include "Graphics/Resource/GraphicsResource.h"
#include <exception>

ID3D11DeviceContext* GraphicsResource::GetContext(Graphics& gfx) noexcept { return gfx.pContext.Get(); }

ID3D11Device* GraphicsResource::GetDevice(Graphics& gfx) noexcept { return gfx.pDevice.Get(); }

DxgiInfoManager& GraphicsResource::GetInfoManager(Graphics& gfx) noexcept(IS_DEBUG) {
	UNREFERENCED_PARAMETER(gfx);
	#ifndef NDEBUG
	return gfx.infoManager;
	#else
	throw std::logic_error("InfoManager is not available in Release configurations.");
	#endif // !NDEBUG
}