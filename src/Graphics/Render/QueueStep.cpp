#include "Graphics/Drawable/Drawable.h"
#include "Graphics/Render/FrameController.h"
#include "Graphics/Render/QueueStep.h"

QueueStep::QueueStep(usize targetPass) : targetPass(targetPass) {}

void QueueStep::AddBindable(std::shared_ptr<Bindable> bind) noexcept {
	bindables.push_back(std::move(bind));
}

void QueueStep::Draw(FrameController& fc, const Drawable& drawable) const {
	fc.Accept(QueueJob{this, &drawable}, targetPass);
}

void QueueStep::Bind(Graphics& gfx) const {
	for (const auto& b : bindables) b->Bind(gfx);
}

void QueueStep::InitializeParentReferences(const Drawable& parent) noexcept {
	for (auto& b : bindables) b->InitializeParentReference(parent);
}

void QueueStep::Accept(TechniqueProbe& probe) {
	probe.SetStep(this);
	for (auto& pb : bindables) pb->Accept(probe);
}
