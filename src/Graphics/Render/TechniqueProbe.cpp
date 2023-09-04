#include "Graphics/ConstantBufferDyn.h"
#include "Graphics/Render/QueueStep.h"
#include "Graphics/Render/Technique.h"
#include "Graphics/Render/TechniqueProbe.h"

void TechniqueProbe::SetTechnique(Technique* pTech_) {
	pTech = pTech_;
	techId++;
	OnSetTechnique();
}

void TechniqueProbe::SetStep(QueueStep* pStep_) {
	pStep = pStep_;
	stepId++;
	OnSetStep();
}

bool TechniqueProbe::VisitBuffer(CBD::Buffer& buf) {
	bufId++;
	return OnVisitBuffer(buf);
}
