#pragma once
#include "TGLib/TGLib.h"

class QueueJob {
public:
	QueueJob(const class QueueStep* pStep, const class Drawable* pDrawable);
	void Execute(class Graphics& gfx) const dbgexcept;
private:
	const class Drawable* pDrawable;
	const class QueueStep* pStep;
};