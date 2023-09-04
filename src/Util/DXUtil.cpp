#include "Util/DXUtil.h"

namespace DXUtil {

DirectX::XMMATRIX CustomOrthoProj(f32 width, f32 height) noexcept {
    return DirectX::XMMATRIX{ 2.0f / width, 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / height, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f };
}

}