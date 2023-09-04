#pragma once

class ImguiManager {
public:
	ImguiManager();
	~ImguiManager();

	bool showUI = true;
private:
	struct ImGuiContext* ctx;
};