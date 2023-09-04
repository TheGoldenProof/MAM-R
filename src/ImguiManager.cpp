#include "ImguiManager.h"
#include "imgui/imgui.h"

ImguiManager::ImguiManager() {
	IMGUI_CHECKVERSION();
	ctx = ImGui::CreateContext();
	ImGui::SetCurrentContext(ctx);
	ImGui::StyleColorsDark();
	ImGui::GetStyle().ScaleAllSizes(2.0f);
}

ImguiManager::~ImguiManager() {
	ImGui::DestroyContext(ctx);
}
