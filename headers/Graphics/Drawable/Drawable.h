#pragma once
#include "Graphics/Graphics.h"
#include "Graphics/Render/Technique.h"

class IndexBuffer;
class VertexBuffer;
class Topology;
class InputLayout;

enum SIZE_MODE {
	SIZE_MODE_SCALE,
	SIZE_MODE_ABSOLUTE,
	SIZE_MODE_LAST,
};

class Drawable {
public:
	Drawable() = default;
	Drawable(const Drawable&) = delete;
	virtual ~Drawable() = default;

	void AddTechnique(Technique tech) noexcept;

	virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;

	void Draw(class FrameController& fc) const noexcept;
	void Bind(Graphics& gfx) const noexcept;
	void Accept(TechniqueProbe& probe);
	u32 GetIndexCount() const dbgexcept;
protected:
	std::shared_ptr<IndexBuffer> pIndicies;
	std::shared_ptr<VertexBuffer> pVerticies;
	std::shared_ptr<Topology> pTopology;
	std::vector<Technique> techniques;
};