#pragma once
#include "TGLib\TGLib.h"
#include <DirectXMath.h>

namespace DXUtil {

DirectX::XMMATRIX CustomOrthoProj(f32 width, f32 height) noexcept;

}