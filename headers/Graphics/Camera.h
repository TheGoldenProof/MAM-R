#pragma once
#include "Graphics/Graphics.h"
#include "Util/Interp.h"
#include "TGLib/Timer.h"
#include <unordered_map>

class Camera {
	friend class CameraScript;
public:
	Camera(const std::string& name, DirectX::XMFLOAT3 homePos = { 0, 0, -480 }) noexcept;
	Camera(const std::string& name, DirectX::XMFLOAT3 homePos, f32 homePitch, f32 homeYaw) noexcept;
	void Bind(Graphics& gfx);
	DirectX::XMMATRIX GetMatrix() const noexcept;
	void Reset() noexcept;
	void Rotate(f32 dx, f32 dy) noexcept;
	void Translate(DirectX::XMFLOAT3 translation) noexcept;
	DirectX::XMFLOAT3 GetPos() const noexcept;
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	void SpawnControlWindow() noexcept;
	const std::string& GetName();
	CameraScript* GetCurrentScript() const noexcept;
private:
	DirectX::XMFLOAT3 homePos;
	f32 homePitch, homeYaw;
	DirectX::XMFLOAT3 pos;
	f32 pitch, yaw;
	std::string name;
	CameraScript* script;
};

class CameraScript {
	friend class CameraArr;
private:
	struct Control {
		f32 startX, startY;
		f32 endX, endY;
		f32 duration;
		Math::INTERP_MODE mode;
	};
public:
	CameraScript(const std::string& path);

	void Update(Camera& cam);

	void ReloadScript() dbgexcept;
	void AttachToCamera(class CameraArr& cams, const std::string& camName) noexcept;
	std::string GetAttachedCamera() const noexcept;

	void Start();
	void SetPaused(bool paused);
	bool IsPaused();
	void Restart();
private:
	bool ReadControl(std::ifstream& file);
private:
	std::string path;
	std::string camName = "";
	u32 active = 0;
	TGLib::Timer timer;
	u32 endBehavior = 0;
	std::vector<Control> controls;
};

class CameraArr {
public:
	void SpawnWindowFor(const std::string& name);
	void AddCamera(std::unique_ptr<Camera>&& cam);
	std::optional<std::reference_wrapper<Camera>> GetCamera(const std::string& name);
	std::optional<std::reference_wrapper<Camera>> GetActiveCamera();
	void SetActive(const std::string& name);
	void BindActive(Graphics& gfx);

	void AddScript(std::unique_ptr<CameraScript>&& script);
	std::optional<std::reference_wrapper<CameraScript>> GetScript(const std::string& path);
	std::optional<std::reference_wrapper<CameraScript>> GetActiveScript();
	void UpdateScripts();
private:
	std::string selected = "";
	std::unordered_map<std::string, std::unique_ptr<Camera>> cameras;
	std::unordered_map<std::string, std::unique_ptr<CameraScript>> scripts;
};