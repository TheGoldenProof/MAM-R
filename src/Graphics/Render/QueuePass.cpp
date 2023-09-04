#include "Graphics/Render/QueuePass.h"

void QueuePass::Accept(const QueueJob& job) noexcept {
	jobs.push_back(job);
}

void QueuePass::Execute(Graphics& gfx) const dbgexcept {
	for (const auto& job : jobs) job.Execute(gfx);
}

void QueuePass::Reset() noexcept {
	jobs.clear();
}
