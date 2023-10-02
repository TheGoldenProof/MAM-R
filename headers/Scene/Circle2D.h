#pragma once
#include "Scene\MidiScene.h"
#include <array>
#include <bitset>

class Circle2D : public MidiScene {
private:
	std::vector<std::unique_ptr<class QuadBatchColored>> trackVisuals;
	std::unique_ptr<class QuadBatchColored> pPlayingVisuals;

	f32 startRadius = 450.0f;
	f32 deltaRadius = -32.0f;
	f32 angleOffset;
	f32 angleRange;
	bool alignEnds = false;
	i32 noteType = 4;
	f32 noteRotation = 0.0f;
	f32 noteSize = 30.0f;
	f32 noteAlpha = 0.125f;
	f32 noteFadeTime = 1.0f;

	std::vector<std::array<f32, 128>> noteFades;
public:
	Circle2D(class Globe& gb, const std::string& name = "Circle 2D");

	virtual void Init(Globe& gb) override;
	virtual void Draw(Globe& gb) override;
protected:
	virtual void InitVisuals(Globe& gb) override;
	virtual void ClearVisuals(Globe& gb) override;
	virtual void MovePlay(Globe& gb, f32 dx) override;
	virtual void DrawGUI(Globe& gb) override;
	virtual void WriteConfig(Globe& gb) override;
	virtual void ReadConfig(Globe& gb) override;
private:
	void UpdateVisuals(Globe& gb);
};