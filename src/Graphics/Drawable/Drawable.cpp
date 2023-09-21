#include "Graphics/Drawable/Drawable.h"
#include "Graphics/GraphicsMacros.h"
#include "Graphics/Bindable/Bindables.h"
#include "Graphics/Bindable/BindableCodex.h"

void Drawable::AddTechnique(Technique tech) noexcept {
	tech.InitializeParentReferences(*this);
	techniques.push_back(std::move(tech));
}

std::vector<Technique>& Drawable::GetTechniques() noexcept {
	return techniques;
}

void Drawable::Draw(FrameController& frame) const noexcept {
	for (const auto& tech : techniques) tech.Draw(frame, *this);
}

void Drawable::Bind(Graphics& gfx) const noexcept {
	pTopology->Bind(gfx);
	pIndicies->Bind(gfx);
	pVerticies->Bind(gfx);
}

void Drawable::Accept(TechniqueProbe& probe) {
	for (Technique& t : techniques) t.Accept(probe);
}

u32 Drawable::GetIndexCount() const dbgexcept {
	return pIndicies->GetCount();
}
