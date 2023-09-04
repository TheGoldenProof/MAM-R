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

void SceneController::SetActive(const std::string& name) dbgexcept {
	auto scene = scenes.find(name);
	if (scene == scenes.end()) {
		DEBUG_LOG(("SceneController::SetActive scene does not exist: " + name).c_str());
		assert(false);
	} else {
		pActive = scene->second.get();
	}
}

void SceneController::Update(Globe& gb) {
	//PROFILE_FUNCTION();
	if (pActive) pActive->Update(gb);
}

void SceneController::Draw(Globe& gb) {
	if (pActive) pActive->Draw(gb);
}
