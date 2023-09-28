#pragma once

#include "omidiv_defs.h"
#include "TGLib\TGLib.h"
#include "TGLib\Timer.h"

class Globe {
	friend class App;
public:
	using DurType = std::chrono::steady_clock::duration;
private:
	class Window* pWnd = nullptr;
	class Graphics* pGraphics = nullptr;
	class SceneController* pSceneCtrl = nullptr;
	class FrameController* pFrameCtrl = nullptr;
	class CameraArr* pCams = nullptr;
	class Keyboard* pKeyboard = nullptr;
	class Mouse* pMouse = nullptr;
	class AudioEngine* pAudio = nullptr;
	class Config* pCfg = nullptr;

	TGLib::Timer lifeTimer;

	u64 tickCount = 0;
	u64 frameCount = 0;

	DurType tickDt{ 0 };
	f32 targetTPS = 0; 
	DurType targetTdt{ 0 };
	f32 drawDt = 0;
	f32 targetFPS = 0; // filled by vsync
	f32 targetFdt = 0;
public:
	Window& Wnd() const { return *pWnd; }
	Graphics& Gfx() const { return *pGraphics; }
	SceneController& SceneCtrl() const { return *pSceneCtrl; }
	FrameController& FrameCtrl() const { return *pFrameCtrl; }
	CameraArr& Cams() const { return *pCams; }
	Keyboard& Kbd() const { return *pKeyboard; }
	Mouse& Mouse() const { return *pMouse; }
	AudioEngine& Audio() const { return *pAudio; }
	Config& Cfg() const { return *pCfg; }

	f32 LifeTime() const noexcept { return lifeTimer.Peek(); }

	u64 TickCount() const noexcept { return tickCount; }
	u64 FrameCount() const noexcept { return frameCount; }

	template<typename Rep = f32, typename Period = std::ratio<1>>
	std::chrono::duration<Rep, Period> RealTickDt_dur() const noexcept { return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(tickDt); }

	template<typename Rep = f32, typename Period = std::ratio<1>>
	Rep RealTickDt() const noexcept { return RealTickDt_dur<Rep, Period>().count(); }

	f32 TargetTPS() const noexcept { return targetTPS; }
	void TargetTPS(f32 target) noexcept;

	template<typename Rep = f32, typename Period = std::ratio<1>>
	std::chrono::duration<Rep, Period> TargetTickDt_dur() const noexcept { return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(targetTdt); }

	template<typename Rep = f32, typename Period = std::ratio<1>>
	Rep TargetTickDt() const noexcept { return TargetTickDt_dur<Rep, Period>().count(); }

	template<typename Rep = f32, typename Period = std::ratio<1>>
	void TargetTickDt(Rep target) noexcept {
		const std::chrono::duration<Rep, Period> _dur(target);
		targetTdt = std::chrono::duration_cast<DurType>(_dur);
		targetTPS = 1.0f / std::chrono::duration_cast<std::chrono::seconds>(_dur).count();
	}

	f32 RealFrameDt() const noexcept { return drawDt; }
	f32 TargetFPS() const noexcept { return targetFPS; }
	void TargetFPS(f32 target) noexcept { targetFPS = target; targetFdt = 1.0f / target; }
	f32 TargetFrameDt() const noexcept { return targetFdt; }
	void TargetFrameDt(f32 target) noexcept { targetFPS = 1.0f / target; targetFdt = target; }

	f32 clearColor[3] = { 0.0f, 0.0f, 0.0f };
};