#pragma once
#include "Scene\MidiScene.h"
#include "Graphics\Bindable\ConstantBuffers.h"
#include <array>
#include <vector>

class Standard3D : public MidiScene {
private:
	std::unique_ptr<PixelConstantBufferCaching> pPlayPlanePBuf;
	std::vector<std::unique_ptr<class QuadBatchColored>> trackVisuals;

	f32 zSpacing = 15.0f;
	f32 zSpacingPrev = 0.0f;
	f32 velocityFactor = 0.75f;
	i32 noteType = 4;
	f32 noteRotation = 0.0f;
	f32 noteHeight = 10.0f;
	f32 noteVSpacing = 2.0f;
	f32 noteHSpacing = 2.0f;
	std::vector<std::array<f32, 4>> trackColors;
public:
	Standard3D(class Globe& gb, const std::string& name = "Standard3D");
protected:
	virtual void Draw(Globe& gb) override;
	virtual void InitVisuals(Globe& gb) override;
	virtual void ClearVisuals(Globe& gb) override;
	virtual void MovePlay(Globe& gb, f32 dx) override;
	virtual void DrawGUI(Globe& gb) override;
	virtual void WriteConfig(Globe& gb) override;
	virtual void ReadConfig(Globe& gb) override;
private:
	void UpdateInputs(Globe& gb);
};
