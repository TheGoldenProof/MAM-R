#include "Graphics/Camera.h"
#include "Util/MyMath.h"
#include "imgui/imgui.h"
#include <algorithm>
#include <fstream>

namespace dx = DirectX;

#pragma region Camera
Camera::Camera(const std::string& name, DirectX::XMFLOAT3 homePos, DirectX::XMFLOAT3 homeRot) noexcept
	: name(name), homePos(homePos), homeRot(homeRot) {
	Reset();
}

void Camera::Bind(Graphics& gfx) {
	gfx.SetCamera(GetMatrix());
}

DirectX::XMMATRIX Camera::GetMatrix() const noexcept {
	using namespace dx;

	const XMVECTOR forwardBase = XMVectorSet(0, 0, 1, 0);
	const XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	const XMVECTOR lookVec = XMVector3Transform(forwardBase, rotMatrix);
	const XMVECTOR upVec = XMVector3Transform(XMVectorSet(0, 1, 0, 0), rotMatrix);
	const XMVECTOR posVec = XMLoadFloat3(&pos);
	const XMVECTOR target = posVec + lookVec;

	return XMMatrixLookAtLH(posVec, target, upVec);
}

void Camera::Reset() noexcept {
	pos = homePos;
	rot = homeRot;
}

void Camera::Rotate(f32 dx, f32 dy) noexcept {
	rot.y = Math::wrapAngle_rad(rot.y + dx);
	rot.x = std::clamp(rot.x + dy, 0.995f * -(f32)Math::HALF_PI, 0.995f * (f32)Math::HALF_PI);
}

void Camera::Translate(DirectX::XMFLOAT3 translation) noexcept {
	using namespace dx;

	XMStoreFloat3(&translation, XMVector3Transform(XMLoadFloat3(&translation), XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z)));
	pos ={pos.x + translation.x, pos.y + translation.y, pos.z + translation.z};
}

DirectX::XMFLOAT3 Camera::GetPos() const noexcept { return pos; }

void Camera::SetPos(DirectX::XMFLOAT3 pos_) noexcept {
	pos = pos_;
}

void Camera::SetHomePos(DirectX::XMFLOAT3 pos_) noexcept {
	homePos = pos_;
}

DirectX::XMFLOAT3 Camera::GetHomePos() const noexcept {
	return homePos;
}

void Camera::SetHomeRotation(DirectX::XMFLOAT3 rotation) noexcept {
	homeRot = rotation;
}

DirectX::XMFLOAT3 Camera::GetHomeRotation() const noexcept {
	return homeRot;
}

void Camera::SpawnControlWindow() noexcept {
	if (ImGui::Begin(name.c_str())) {
		ImGui::Text("Position");
		ImGui::InputFloat("X", &pos.x, 20.0f, 200.0f);
		ImGui::InputFloat("Y", &pos.y, 20.0f, 200.0f);
		ImGui::InputFloat("Z", &pos.z, 20.0f, 200.0f);
		if (ImGui::Button("Set as default##pos")) {
			homePos = pos;
		}
		DirectX::XMFLOAT3 rotDeg = {Math::to_deg(rot.x), Math::to_deg(rot.y), Math::to_deg(rot.z)};
		const char* fmt = "%.1f";
		ImGui::Text("Orientation");
		ImGui::SetNextItemWidth(48.0f);
		if (ImGui::InputFloat("##Pitchinp", &rotDeg.x, 0, 0, fmt))
			rot.x = Math::to_rad(Math::wrapAngle_deg(rotDeg.x));
		ImGui::SameLine();
		ImGui::SliderAngle("Pitch", &rot.x, -179.0f, 179.0f);
		ImGui::SetNextItemWidth(48.0f);
		if (ImGui::InputFloat("##Yawinp", &rotDeg.y, 0, 0, fmt))
			rot.y = Math::to_rad(std::clamp(Math::wrapAngle_deg(rotDeg.y), -179.0f, 179.0f));
		ImGui::SameLine();
		ImGui::SliderAngle("Yaw", &rot.y, -180.0f, 180.0f);
		ImGui::SetNextItemWidth(48.0f);
		if (ImGui::InputFloat("##Rollinp", &rotDeg.z, 0, 0, fmt))
			rot.z = Math::to_rad(Math::wrapAngle_deg(rotDeg.z));
		ImGui::SameLine();
		ImGui::SliderAngle("Roll", &rot.z, -180.0f, 180.0f);
		if (ImGui::Button("Set as default##rot")) {
			homeRot = rot;
		}
	}
	ImGui::End();
}

const std::string& Camera::GetName() {
	return name;
}
CameraScript* Camera::GetCurrentScript() const noexcept {
	return script;
}
#pragma endregion



#pragma region CameraScript
CameraScript::CameraScript(const std::string& path) : path("assets/scripts/" + path) {
	timer.SetPaused(true);
	ReloadScript();
}

void CameraScript::Update(Camera& cam) {
	if (IsPaused()) return;

	Control ctrl = controls[active];

	while (timer.Peek() >= ctrl.duration) {
		active++;
		if (active == controls.size()) {
			if (endBehavior == 0) {
				SetPaused(true);
				Restart();
				return;
			} else if (endBehavior == 1) {
				active = 0;
			}
		}
		ctrl = controls[active];
		timer.Mark();
	}

	cam.SetPos({
		Math::interpolate(ctrl.mode, ctrl.startX, ctrl.endX, timer.Peek() / ctrl.duration),
		Math::interpolate(ctrl.mode, ctrl.startY, ctrl.endY, timer.Peek() / ctrl.duration),
		0.0f });
}

void CameraScript::AttachToCamera(CameraArr& cams, const std::string& name) noexcept {
	camName = name;
	if (auto cam = cams.GetCamera(name); cam) {
		cam.value().get().script = this;
	}
}

std::string CameraScript::GetAttachedCamera() const noexcept {
	return camName;
}

void CameraScript::Start() {
	timer.Mark();
	timer.SetPaused(false);
}

void CameraScript::SetPaused(bool paused) {
	timer.SetPaused(paused);
}

bool CameraScript::IsPaused() {
	return timer.IsPaused();
}

void CameraScript::Restart() {
	active = 0;
	timer.Mark();
}

void CameraScript::ReloadScript() dbgexcept {
	std::ifstream file(path);
	if (!file.is_open()) {
		DEBUG_LOG(("Couldn't open camera script: " + path).c_str());
		assert(false);
		return;
	}

	controls.clear();

	file >> endBehavior;

	while (!file.eof() && ReadControl(file));

	if (controls.size() == 0) {
		DEBUG_LOG(("Script has no controls: " + path + "\n").c_str());
		assert(false);
		return;
	}
}

#define EX(type, name) type name; if(!(file >> name)) return false

bool CameraScript::ReadControl(std::ifstream& file) {
	EX(f32, startX);
	EX(f32, startY);
	EX(f32, endX);
	EX(f32, endY);
	EX(f32, duration);
	EX(u32, mode);

	controls.push_back(Control{startX, startY, endX, endY, duration, (Math::INTERP_MODE)mode});
	return true;
}

#undef EX
#pragma endregion



#pragma region CameraArr
void CameraArr::SpawnWindowFor(const std::string& name) {
	if (auto cam = GetCamera(name); cam) {
		cam.value().get().SpawnControlWindow();
	}
}

void CameraArr::AddCamera(std::unique_ptr<Camera>&& cam) {
	cameras[cam->GetName()] = std::move(cam);
}

std::optional<std::reference_wrapper<Camera>> CameraArr::GetCamera(const std::string& name) {
	if (auto i = cameras.find(name); i != cameras.end()) {
		return { std::ref(*i->second) };
	}
	return {};
}

std::optional<std::reference_wrapper<Camera>> CameraArr::GetActiveCamera() {
	return GetCamera(selected);
}

void CameraArr::SetActive(const std::string& name) {
	selected = name;
}

void CameraArr::BindActive(Graphics& gfx) {
	if (auto cam = GetActiveCamera(); cam) cam.value().get().Bind(gfx);
}

void CameraArr::AddScript(std::unique_ptr<CameraScript>&& script) {
	scripts[script->path] = std::move(script);
}

std::optional<std::reference_wrapper<CameraScript>> CameraArr::GetScript(const std::string& path) {
	if (auto i = scripts.find(path); i != scripts.end()) {
		return { std::ref(*i->second) };
	}
	return {};
}

std::optional<std::reference_wrapper<CameraScript>> CameraArr::GetActiveScript() {
	if (auto cam = GetActiveCamera(); cam) {
		if (auto script = cam.value().get().GetCurrentScript(); script) {
			return { std::ref(*script) };
		}
	}
	return {};
}

void CameraArr::UpdateScripts() {
	for (auto it = scripts.begin(); it != scripts.end(); it++) {
		auto& pScript = it->second;
		if (auto cam = GetCamera(it->first); cam)
			pScript->Update(cam.value().get());
	}
}
#pragma endregion
