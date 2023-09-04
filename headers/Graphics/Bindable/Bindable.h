#pragma once
#include "Graphics/Resource/GraphicsResource.h"
#include "Graphics/Render/TechniqueProbe.h"

class Bindable : public GraphicsResource {
public:
	virtual void Bind(Graphics& gfx) noexcept = 0;
	virtual ~Bindable() = default;

	virtual void InitializeParentReference(const class Drawable&) noexcept {}
	virtual void Accept(TechniqueProbe&) {}
	virtual std::string GetUID() const noexcept = 0;
};

class BindableCloning : public Bindable {
public:
	virtual std::unique_ptr<BindableCloning> Clone() const noexcept = 0;
};