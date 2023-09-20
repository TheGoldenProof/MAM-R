#pragma once
#include "MIDI\CookedMidi.h"
#include "miniaudio\Sound.h"
#include "Scene\Scene.h"

class MidiScene : public Scene {
protected:
	struct color_ {
		u8 r, g, b, a;
	};
	static constexpr color_ colors[] = {
		{255, 0, 0, 255},
		{255, 127, 0, 255},
		{255, 191, 0, 255},
		{255, 255, 0, 255},
		{191, 255, 0, 255},
		{127, 255, 0, 255},
		{63, 255, 0, 255},
		{0, 255, 0, 255},
		{0, 255, 63, 255},
		{0, 255, 127, 255},
		{0, 255, 191, 255},
		{0, 255, 255, 255},
		{0, 191, 255, 255},
		{0, 127, 255, 255},
		{0, 63, 255, 255},
		{0, 0, 255, 255},
		{63, 0, 255, 255},
		{127, 0, 255, 255},
		{191, 0, 255, 255},
		{255, 0, 255, 255},
		{255, 0, 191, 255},
		{255, 0, 127, 255},
		{255, 0, 63, 255},
	};
	static constexpr f32 noteScaleFactor = 1.0f / 1.5f;
protected:
	std::unordered_set<std::unique_ptr<Drawable>> visuals;
	bool needsReset = false;

	std::wstring midiPath;
	MIDI::CookedMidi midi;
	u32 currentTick = 0;
	u32 tempoMicros = 500000;
	u32 tempoMapIndex = 0;
	bool isPlaying = false;
	bool reloadMidi = false;

	std::wstring audioPath;
	Sound sound;
	bool reloadAudio = false;
	f32 volume = 1.0f;
	f32 volumePrev = 1.0f;
	i32 audioOffset = 0;
	i32 audioOffsetPrev = 0;

	class QuadColored* pPlayLine = nullptr;
public:
	MidiScene(class Globe& gb, const std::string& name);

	virtual void Init(Globe& gb) override;
	virtual void Update(Globe& gb) override;
	virtual void Draw(Globe& gb) override;

	virtual void Reset(Globe& gb);
protected:
	virtual void ClearVisuals(Globe& gb);
	virtual void InitVisuals(Globe& gb) = 0;
private:
	void InitMidi(class Globe& gb);
	void UpdateTPS(class Globe& gb);
	void DrawGUI(class Globe& gb);
};