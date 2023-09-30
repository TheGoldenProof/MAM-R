// OMIDIV.cpp : Defines the entry point for the application.
//

#include "Graphics\Bindable\Bindables.h"
#include "imgui\imgui.h"
#include "Scene\Scenes.h"
#include "Windows/framework.h"
#include "../resource.h"
#include "OMIDIV.h"
#include "Util\DXUtil.h"
#include "Util\MyMath.h"
#include <numeric>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    i32 ret = -1;
    try {
        ret = App(hInstance).Run();
    } catch (const MyException& e) {
        DEBUG_LOG("\n\n");
        DEBUG_LOG(e.what());
        DEBUG_LOG("\n\n");
        MessageBoxA(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
    } catch (const std::exception& e) {
        DEBUG_LOG("\n\n");
        DEBUG_LOG(e.what());
        DEBUG_LOG("\n\n");
        MessageBoxA(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
    } catch (...) {
        MessageBoxA(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
    }
    return ret;
}

App::App(HINSTANCE hInstance) : hInstance(hInstance), wnd(OMIDIV_WINW, OMIDIV_WINH, L""), frameCtrl(wnd.Gfx()) {

    WCHAR wndTitle[OMIDIV_MAX_LOADSTRING];
    LoadStringW(hInstance, IDS_APP_TITLE, wndTitle, OMIDIV_MAX_LOADSTRING);

    gb.pWnd = &wnd;
    gb.pGraphics = &wnd.Gfx();
    gb.pSceneCtrl = &sceneCtrl;
    gb.pFrameCtrl = &frameCtrl;
    gb.pCams = &cams;
    gb.pKeyboard = &wnd.kbd;
    gb.pMouse = &wnd.mouse;
    gb.pAudio = &audioEngine;
    gb.pCfg = &cfg;
    gb.TargetFPS(gb.Gfx().GetRefreshRate());
    gb.TargetTPS(gb.TargetFPS());

    wnd.SetTitle(wndTitle);

    gb.Cams().AddCamera(std::make_unique<Camera>("Camera0"));
    gb.Cams().AddCamera(std::make_unique<Camera>("HUD cam"));
    gb.Cams().SetActive("Camera0");

    sceneCtrl.AddScene(std::make_unique<Standard3D>(gb));
    sceneCtrl.SetActive(gb, "Standard3D");

    //auto proj = DXUtil::CustomOrthoProj(OMIDIV_WINW, OMIDIV_WINH);
}

App::~App() {}

i32 App::Run() {
    Blender::Resolve(wnd.Gfx(), true, {})->Bind(gb.Gfx());
    Rasterizer::Resolve(wnd.Gfx(), false)->Bind(gb.Gfx());

    TGLib::Timer mainTimer;
    TGLib::Timer windowTimer;
    TGLib::Timer updateTimer;
    TGLib::Timer drawTimer;
    f32 updateTime_ = 0.0f;
    while (true) {

        if (windowTimer.Peek() >= gb.TargetFrameDt()) {
            if (const auto ecode = Window::ProcessMessages()) return *ecode;
            windowTimer.Mark();
        }

        // this is placed after message processing because ProcessMessages may block (such as when dragging the window).
        updateTime_ += mainTimer.Mark();

        /* This uses a different method of tracking to allow for a hybrid fixed/non-fixed update.
        * For example, grabbing the title bar (to drag the window) will cause ProcessMessage to block.
        * In this case, it may block for, example, 5 ticks. The next time the time since the last update is checked,
        * it will update. If the update clock is reset to zero, then the game will only update once at the
        * end of the stall, and so 5 tick would be missed. But with this, it will continue to call update in
        * succession until it's back on track. So, at the end of the stall, it will call update 5 times in a row.
        * If you use fixed time in your update function (gb.TargetTickDt()), update will be called
        * 5 times within the span that 5 ticks should occur. If you use variable time in your update
        * function (gb.tickDt), you will get one large dt the length of 5 ticks followed by 4 short near-zero dt's.
        * The draw function doesn't need this, because it doesn't need to redraw the screen 5 times in rapid
        * succession, because only the last one is what actually gets displayed.
        */
        if (updateTime_ >= gb.TargetTickDt()) {
            updateTime_ -= gb.TargetTickDt();
            Update(updateTimer.Mark_dur<i64, std::nano>());
        }

        if (drawTimer.Peek() >= gb.TargetFrameDt()) {
            Draw(drawTimer.Mark());
        }

    }
}

void App::Update(std::chrono::steady_clock::duration dt) {
    gb.tickDt = dt;
    gb.tickCount++;

    tpsArr[tpsI] = std::chrono::duration<f32>(dt).count();
    tpsI = (tpsI + 1) % tpsArr.size();

    sceneCtrl.Update(gb);
}

void App::Draw(f32 dt) {
    gb.drawDt = dt;
    gb.frameCount++;

    fpsArr[fpsI] = dt;
    fpsI = (fpsI + 1) % fpsArr.size();

    if (gb.Kbd().KeyPressed(VK_F1)) {
        gb.Gfx().SetImguiEnabled(!gb.Gfx().IsImguiEnabled());
    }

    if (gb.Kbd().KeyPressed(VK_F3)) {
        gb.Gfx().SetImDebugEnabled(!gb.Gfx().IsImDebugEnabled());
    }

    if (gb.Kbd().KeyPressed(VK_F11)) {
        wnd.SetBorderlessFullscreen(!wnd.IsFullscreen());
    }

    gb.Gfx().BeginFrame();
    gb.Gfx().ClearBuffer(gb.clearColor[0], gb.clearColor[1], gb.clearColor[2]);
    gb.Gfx().drawCalls = 0;

    gb.Cams().BindActive(gb.Gfx());

    //if (show_demo_window && gb.Gfx().IsImguiEnabled()) 
    //ImGui::ShowDemoWindow();
    if (gb.Gfx().IsImguiEnabled()) gb.Cams().SpawnWindowFor("Camera0");

    if (gb.Gfx().IsImDebugEnabled()) {
        f32 avgTps = std::accumulate(tpsArr.begin(), tpsArr.end(), 0.0f) / tpsArr.size();
        f32 avgFps = std::accumulate(fpsArr.begin(), fpsArr.end(), 0.0f) / fpsArr.size();

        f32 nww = ImGui::GetFontSize() * 12.0f;
        f32 nwh = ImGui::GetFontSize() * 6.0f;
        ImGui::SetNextWindowSize({ nww, nwh });
        ImGui::SetNextWindowPos({ gb.Gfx().GetWidth() - nww, gb.Gfx().GetHeight() - nwh });
        if (ImGui::Begin("fps", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs)) {
            //ImGui::SetWindowFontScale(2.0f);
            ImGui::Text("%02.2f tps", 1.0f / avgTps);
            ImGui::Text("%02.2f fps", 1.0f / avgFps);
        }
        ImGui::End();

        nww = ImGui::GetFontSize() * 24.0f;
        nwh = ImGui::GetFontSize() * 6.0f;
        ImGui::SetNextWindowSize({ nww, nwh });
        ImGui::SetNextWindowPos({ 0, 0 });
        if (ImGui::Begin("debug", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs)) {
            //ImGui::SetWindowFontScale(2.0f);
            ImGui::Text("draws: %d", gb.Gfx().drawCalls);
        }
        ImGui::End();
    }

    // Draw stuff here
    sceneCtrl.Draw(gb);
    //
    frameCtrl.Execute(gb);

    gb.Gfx().EndFrame();
    gb.Kbd().Update();
    frameCtrl.Reset();
}
