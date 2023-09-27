#pragma once

#include "Globe.h"
#include "Graphics\Camera.h"
#include "Graphics\Render\FrameController.h"
#include "ImguiManager.h"
#include "miniaudio\AudioEngine.h"
#include "Scene\SceneController.h"
#include "TGLib/TGLib.h"
#include "Windows/Window.h"
#include "Config.h"

class App {
public:
	App(HINSTANCE hInstance);
	~App();

	i32 Run();
private:
	void Update(std::chrono::steady_clock::duration dt);
	void Draw(f32 dt);
private:
	Globe gb;

	ImguiManager imgui;
	HINSTANCE hInstance;
	Window wnd;
	CameraArr cams;
	FrameController frameCtrl;
	SceneController sceneCtrl;
	AudioEngine audioEngine;
	Config cfg;

	std::array<f32, 60> tpsArr = {};
	usize tpsI = 0;
	std::array<f32, 60> fpsArr = {};
	usize fpsI = 0;
};