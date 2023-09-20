#pragma once
#include "Scene\MidiScene.h"

class TestScene : public MidiScene {
public:
	TestScene(class Globe& gb);
protected:
	virtual void Draw(Globe& gb) override;
	virtual void InitVisuals(Globe& gb) override;
private:
	void UpdateInputs(Globe& gb);
};
