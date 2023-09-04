#pragma once
#include "Graphics/Render/QueueStep.h"
#include "Graphics/Render/TechniqueProbe.h"
#include <vector>

class Technique {
public:
	Technique() = default;
	Technique(std::string name) noexcept;

	void Draw(class FrameController& fc, const class Drawable& drawable) const noexcept;
	void AddStep(const QueueStep& step) noexcept;
	bool IsActive() const noexcept;
	void SetActive(bool active) noexcept;
	const std::string& GetName() const noexcept;
	void InitializeParentReferences(const class Drawable& parent) noexcept;
	void Accept(TechniqueProbe& probe);
private:
	bool active = true;
	std::vector<QueueStep> steps;
	std::string name = "Unnamed";
};