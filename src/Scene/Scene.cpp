#include "Globe.h"
#include "Graphics\Drawable\Drawables.h"
#include "Scene\Scene.h"
#include <fstream>

Scene::Scene(Globe& gb, const std::string& name) : name(name) {
	UNREFERENCED_PARAMETER(gb);
}

std::string Scene::GetName() const noexcept {
	return name;
}

void Scene::Init(Globe& gb) {
	UNREFERENCED_PARAMETER(gb);
}

void Scene::Update(Globe& gb) {
	UNREFERENCED_PARAMETER(gb);
}

void Scene::Draw(Globe& gb) {
	for (auto& drawable : drawables) {
		drawable->Draw(gb.FrameCtrl());
	}
}

void Scene::Denit(Globe& gb) {
	UNREFERENCED_PARAMETER(gb);
}

void Scene::LoadFile(Globe& gb, const std::string& path_) dbgexcept {
	path = path_;
	if (path.empty()) return;
	
	std::string fullPath = "assets/" + path;
	std::ifstream file(fullPath);
	if (!file.is_open()) {
		DEBUG_LOG(("Couldn't open scene: " + fullPath).c_str());
		assert(false && "Couldn't open scene file");
		return;
	}

	drawables.clear();

	while (!file.eof() && ReadObject(gb, file));
}

void Scene::Reload(Globe& gb) dbgexcept {
	LoadFile(gb, path);
}

#define EX(type, name) type name; if(!(file >> name)) return false

bool Scene::ReadObject(Globe& gb, std::ifstream& file) {
	EX(u32, type);

	switch (type) {
	case 0:
		return ReadQuad(gb.Gfx(), file);
	default:
		DEBUG_LOG(("Invalid object type: " + std::to_string(type)).c_str());
		assert(false);
	}

	return true;
}

bool Scene::ReadQuad(Graphics& gfx, std::ifstream& file) {
	EX(u32, layer);
	EX(std::string, tex);
	EX(f32, sizeX);
	EX(f32, sizeY);
	EX(f32, posX);
	EX(f32, posY);
	EX(u32, sizeMode);
	assert(sizeMode < SIZE_MODE_LAST);
	EX(f32, offsetU);
	EX(f32, offsetV);
	EX(f32, sizeU);
	EX(f32, sizeV);
	EX(u32, filter);
	EX(std::string, vs);
	EX(std::string, ps);

	if (vs == "n") vs = "VS_Texture.cso";
	if (ps == "n") ps = "PS_Texture.cso";

	auto quad = std::make_unique<Quad>(gfx,
		Quad::QuadDesc{name +"_q" + std::to_string(drawables.size()), tex, {sizeX, sizeY}, layer, (SIZE_MODE)sizeMode,
		{offsetU, offsetV}, {sizeU, sizeV}, (D3D11_FILTER)filter, vs, ps });
	quad->SetPos({ posX, posY, 0.0f });

	drawables.insert(std::move(quad));

	return true;
}

#undef EX