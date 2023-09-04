#pragma once
#include "Graphics/Graphics.h"
#include "Graphics/Render/QueueJob.h"
#include <vector>

class QueuePass {
public:
	void Accept(const QueueJob& job) noexcept;
	void Execute(Graphics& gfx) const dbgexcept;
	void Reset() noexcept;
private:
	std::vector<QueueJob> jobs;
};