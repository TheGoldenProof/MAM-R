#include "Globe.h"

void Globe::TargetTPS(f32 target) noexcept { 
	targetTPS = target; 
	targetTdt = std::chrono::duration_cast<DurType>(std::chrono::duration<f32>(1.0f / target));
}