#pragma once

namespace CBD {
class Buffer;
}
class Technique;
class QueueStep;

class TechniqueProbe {
public:
	virtual ~TechniqueProbe() {}

	void SetTechnique(Technique* pTech);
	void SetStep(QueueStep* pStep);
	bool VisitBuffer(CBD::Buffer&);
protected:
	virtual void OnSetTechnique() {}
	virtual void OnSetStep() {}
	virtual bool OnVisitBuffer(CBD::Buffer&) = 0;
protected:
	Technique* pTech = nullptr;
	QueueStep* pStep = nullptr;
	usize techId = ULLONG_MAX;
	usize stepId = ULLONG_MAX;
	usize bufId = ULLONG_MAX;
};