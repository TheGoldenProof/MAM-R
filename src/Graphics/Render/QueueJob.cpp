#include "Graphics/Drawable/Drawable.h"
#include "Graphics/Render/QueueJob.h"
#include "Graphics/Render/QueueStep.h"

QueueJob::QueueJob(const QueueStep* pStep, const Drawable* pDrawable) : pStep(pStep), pDrawable(pDrawable) {}

void QueueJob::Execute(Graphics& gfx) const dbgexcept {
	pDrawable->Bind(gfx);
	pStep->Bind(gfx);
	gfx.DrawIndexed(pDrawable->GetIndexCount());
}
