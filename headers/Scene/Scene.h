#pragma once
#include "Graphics\Drawable\Drawable.h"
#include <unordered_set>

class Globe;

class Scene {
protected:
	std::string name;
	std::string path;
	std::unordered_set<std::unique_ptr<Drawable>> drawables;
public:
	Scene(Globe& gb, const std::string& name);
	virtual ~Scene() {}

	std::string GetName() const noexcept;

	virtual void Init(Globe& gb);
	virtual void Update(Globe& gb);
	virtual void Draw(Globe& gb);
	virtual void Denit(Globe& gb);

	void LoadFile(Globe& gb, const std::string& path) dbgexcept; // does nothing if path is empty, throws (dbg only) if invalid.
	void Reload(Globe& gb) dbgexcept;
protected:
	bool ReadObject(Globe& gb, std::ifstream& file);
	bool ReadQuad(Graphics& gfx, std::ifstream& file);
};