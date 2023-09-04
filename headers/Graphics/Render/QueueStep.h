#pragma once
#include "Graphics/Bindable/Bindable.h"
#include "Graphics/Graphics.h"
#include "Graphics/Render/TechniqueProbe.h"
#include <vector>
#include <memory>

class QueueStep {
public:
	QueueStep(usize targetPass);
	void AddBindable(std::shared_ptr<Bindable> bind) noexcept;
	void Draw(class FrameController& fc, const class Drawable& drawable) const;
	void Bind(Graphics& gfx) const;
	void InitializeParentReferences(const class Drawable& parent) noexcept;
	void Accept(TechniqueProbe& probe);
private:
	usize targetPass;
	std::vector<std::shared_ptr<Bindable>> bindables;
};