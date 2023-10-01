#include "MyException.h"
#include "Scene\SceneController.h"

void SceneController::AddScene(std::unique_ptr<Scene>&& scene) noexcept {
	scenes[scene->GetName()] = std::move(scene);
}

Scene* SceneController::GetScene(const std::string& name) {
	auto ret = scenes.find(name);
	return (ret == scenes.end()) ? nullptr : ret->second.get();
}

Scene* SceneController::GetActive() {
	return pActive;
}

std::vector<std::string> SceneController::GetSceneNames() const noexcept {
	std::vector<std::string> ret; ret.reserve(scenes.size());
	for (const auto& p : scenes) ret.push_back(p.first);
	return ret;
}

void SceneController::SetActive(Globe& gb, const std::string& name) dbgexcept {
	auto scene = scenes.find(name);
	if (scene == scenes.end()) {
		DEBUG_LOG(("SceneController::SetActive scene does not exist: " + name).c_str());
		assert(false);
	} else {
		if (pActive) pActive->Denit(gb);
		pActive = scene->second.get();
		if (pActive) pActive->Init(gb);
	}
}

void SceneController::SetNextActive(const std::string& name) noexcept {
	nextActive = name;
}

void SceneController::Init(Globe& gb) {
	if (pActive) pActive->Init(gb);
}

void SceneController::Update(Globe& gb) {
	if (!nextActive.empty()) {
		SetActive(gb, nextActive);
		nextActive.clear();
	}
	if (pActive) pActive->Update(gb);
}

void SceneController::Draw(Globe& gb) {
	if (pActive) pActive->Draw(gb);
}

void SceneController::Denit(Globe& gb) {
	if (pActive) pActive->Denit(gb);
}
