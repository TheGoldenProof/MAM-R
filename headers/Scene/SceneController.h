#pragma once
#include "Graphics\Camera.h"
#include "Scene.h"
#include "TGLib\TGLib.h"
#include <unordered_map>

class SceneController {
public:
	SceneController() = default;
	SceneController(const SceneController&) = delete;
	SceneController& operator=(const SceneController&) = delete;
	~SceneController() = default;

	void AddScene(std::unique_ptr<Scene>&& scene) noexcept;
	Scene* GetScene(const std::string& name);
	void SetNextActive(const std::string& name) noexcept;
	Scene* GetActive();

	std::vector<std::string> GetSceneNames() const noexcept;

	void Init(class Globe& gb);
	void Update(class Globe& gb);
	void Draw(class Globe& gb);
	void Denit(class Globe& gb);
private:
	void SetActive(Globe& gb, const std::string& name) dbgexcept;

	std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
	Scene* pActive = nullptr;
	std::string nextActive;
};