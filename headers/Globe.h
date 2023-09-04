#pragma once

#include "mamr_defs.h"
#include "TGLib\TGLib.h"
#include "TGLib\Timer.h"

class Globe {
	friend class App;
private:
	class Graphics* pGraphics = nullptr;
	//class SceneController* pSceneCtrl = nullptr;
	class FrameController* pFrameCtrl = nullptr;
	class CameraArr* pCams = nullptr;
	class Keyboard* pKeyboard = nullptr;
	class Mouse* pMouse = nullptr;

	TGLib::Timer lifeTimer;

	u64 tickCount = 0;
	u64 frameCount = 0;

	f32 tickDt = 0;
	f32 targetTPS = 0; 
	f32 targetTdt = 0;
	f32 drawDt = 0;
	f32 targetFPS = 0; // filled by vsync
	f32 targetFdt = 0;
public:
	Graphics& Gfx() const { return *pGraphics; }
	//SceneController& SceneCtrl() const { return *pSceneCtrl; }
	FrameController& FrameCtrl() const { return *pFrameCtrl; }
	CameraArr& Cams() const { return *pCams; }
	Keyboard& Kbd() const { return *pKeyboard; }
	Mouse& Mouse() const { return *pMouse; }

	f32 LifeTime() const noexcept { return lifeTimer.Peek(); }

	u64 TickCount() const noexcept { return tickCount; }
	u64 FrameCount() const noexcept { return frameCount; }

	f32 RealTickDt() const noexcept { return tickDt; }
	f32 TargetTPS() const noexcept { return targetTPS; }
	void TargetTPS(f32 target) noexcept { targetTPS = target; targetTdt = 1.0f / target; }
	f32 TargetTickDt() const noexcept { return targetTdt; }
	void TargetTickDt(f32 target) noexcept { targetTPS = 1.0f / target; targetTdt = target; }

	f32 RealFrameDt() const noexcept { return drawDt; }
	f32 TargetFPS() const noexcept { return targetFPS; }
	void TargetFPS(f32 target) noexcept { targetFPS = target; targetFdt = 1.0f / target; }
	f32 TargetFrameDt() const noexcept { return targetFdt; }
	void TargetFrameDt(f32 target) noexcept { targetFPS = 1.0f / target; targetFdt = target; }

};