#pragma once
#include "Graphics\Bindable\ConstantBuffers.h"
#include "Graphics\Drawable\Quad.h"
#include "MIDI\CookedMidi.h"
#include "miniaudio\Sound.h"
#include "Scene\Scene.h"
#include <chrono>

class MidiScene : public Scene {
private:
	MIDI::RawMidi rawMidi;
	bool midiPathChanged = false;
protected:
	bool autoReload = true;
	bool needsReset = false;
	bool needsRestart = false;
	std::chrono::steady_clock::time_point lastValueChange;

	u32 currentTick = 0;
	std::chrono::microseconds currentTime{ 0 };
	u32 currentTempo = 500000;

	f32 lengthScale = 2.0f/3.0f;
	std::wstring midiPath;
	MIDI::CookedMidi midi;
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

	std::wstring imagePath;
	std::unique_ptr<QuadTextured> pBGImg;
	std::unique_ptr<PixelConstantBufferCaching> pBGImgTintBuf;
	DirectX::XMFLOAT4 bgImgTint = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool clearBGImg = false;

	f32 playX = 0.0f;
	std::vector<std::pair<usize, bool>> trackReorder; // <index, enabled>
public:
	MidiScene(class Globe& gb, const std::string& name);

	virtual void Init(Globe& gb) override;
	virtual void Update(Globe& gb) override;
	virtual void Draw(Globe& gb) override;

	virtual void Restart(Globe& gb);
	virtual void Reset(Globe& gb);
protected:
	virtual void InitVisuals(Globe& gb) = 0;
	virtual void ClearVisuals(Globe& gb) = 0;
	virtual void MovePlay(Globe& gb, f32 dx) = 0;
	virtual void DrawGUI(class Globe& gb);
	virtual void WriteConfig(Globe& gb);
	virtual void ReadConfig(Globe& gb);
	f32 MicrosToPixels(i64 start, i64 micros) const;
private:
	void InitMidi(class Globe& gb);
	void UpdateTPS(class Globe& gb);
};