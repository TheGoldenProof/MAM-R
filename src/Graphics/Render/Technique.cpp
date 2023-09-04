#include "Graphics/Drawable/Drawable.h"
#include "Graphics/Render/FrameController.h"
#include "Graphics/Render/Technique.h"

Technique::Technique(std::string name) noexcept : name(name) {}

void Technique::Draw(FrameController& fc, const Drawable& drawable) const noexcept {
	if (active) {
		for (const auto& step : steps) step.Draw(fc, drawable);
	}
}

void Technique::AddStep(const QueueStep& step) noexcept {
	steps.push_back(step);
}

bool Technique::IsActive() const noexcept { return active; }
void Technique::SetActive(bool active_) noexcept { active = active_; }
const std::string& Technique::GetName() const noexcept { return name; }

void Technique::InitializeParentReferences(const Drawable& parent) noexcept {
	for (auto& s : steps) s.InitializeParentReferences(parent);
}

void Technique::Accept(TechniqueProbe& probe) {
	probe.SetTechnique(this);
	for (QueueStep& s : steps) s.Accept(probe);
}
