#pragma once
#include "MIDI\CookedMidi.h"
#include "miniaudio\Sound.h"
#include "Scene\Scene.h"

class MidiScene : public Scene {
protected:
	bool needsReset = false;

	f32 lengthScale = 2.0f/3.0f;
	std::wstring midiPath;
	MIDI::CookedMidi midi;
	u32 currentTick = 0;
	u32 tempoMicros = 500000;
	u32 tempoMapIndex = 0;
	bool isPlaying = false;
	bool reloadMidi = false;
	i32 midiOffset = 0;
	i32 midiOffsetPrev = 0;

	std::wstring audioPath;
	Sound sound;
	bool reloadAudio = false;
	f32 volume = 1.0f;
	f32 volumePrev = 1.0f;
	i32 audioOffset = 0;
	i32 audioOffsetPrev = 0;

	f32 playX = 0.0f;
public:
	MidiScene(class Globe& gb, const std::string& name);

	virtual void Init(Globe& gb) override;
	virtual void Update(Globe& gb) override;
	virtual void Draw(Globe& gb) override;

	virtual void Reset(Globe& gb);
protected:
	virtual void InitVisuals(Globe& gb) = 0;
	virtual void ClearVisuals(Globe& gb) = 0;
	virtual void MovePlay(Globe& gb, f32 dx) = 0;
	virtual void DrawGUI(class Globe& gb);
private:
	void InitMidi(class Globe& gb);
	void UpdateTPS(class Globe& gb);
};