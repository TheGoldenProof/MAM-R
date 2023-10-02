#pragma once
#include "Graphics\Bindable\ConstantBuffers.h"
#include "Graphics\Drawable\Quad.h"
#include "MIDI\CookedMidi.h"
#include "miniaudio\Sound.h"
#include "Scene\Scene.h"
#include <chrono>

class MidiScene : public Scene {
private:
	static MIDI::RawMidi rawMidi;
	static bool midiPathChanged;
protected:
	bool isPlaying = false;

	bool autoReload = true;
	bool needsReset = false;
	bool needsRestart = false;
	bool reloadMidi = false;
	bool reloadVisuals = false;
	bool reloadAudio = false;
	std::chrono::steady_clock::time_point lastValueChange;

	u32 currentTick = 0;
	std::chrono::microseconds currentTime{ 0 };
	u32 currentTempo = 500000;

	f32 lengthScale = 2.0f/3.0f;
	static std::wstring midiPath;
	static MIDI::CookedMidi midi;
	static i32 midiOffset;
	static i32 midiOffsetPrev;

	static std::wstring audioPath;
	static Sound sound;
	static f32 volume;
	static f32 volumePrev;
	static i32 audioOffset;
	static i32 audioOffsetPrev;

	static std::wstring imagePath;
	static std::unique_ptr<QuadTextured> pBGImg;
	static std::unique_ptr<PixelConstantBufferCaching> pBGImgTintBuf;
	static DirectX::XMFLOAT4 bgImgTint;
	bool clearBGImg = false;

	static f32 playX;
	static std::vector<std::pair<usize, bool>> trackReorder; // <index, enabled>

	static std::unordered_map<std::string, bool> sceneSel;
	f32 camFOV = 60.0f;

	static std::vector<std::array<f32, 4>> trackColors;

public:
	MidiScene(class Globe& gb, const std::string& name);

	virtual void Init(Globe& gb) override;
	virtual void Update(Globe& gb) override;
	virtual void Draw(Globe& gb) override;
	virtual void Denit(Globe& gb) override;

	virtual void Restart(Globe& gb);
	virtual void Reset(Globe& gb);
protected:
	virtual void InitVisuals(Globe& gb) = 0;
	virtual void ClearVisuals(Globe& gb) = 0;
	virtual void MovePlay(Globe& gb, f32 dx) = 0;
	virtual void DrawGUI(class Globe& gb);
	virtual void WriteConfig(Globe& gb);
	virtual void ReadConfig(Globe& gb);
	f32 MicrosToTicks(i64 start, i64 micros) const;
private:
	void InitMidi(class Globe& gb);
	void UpdateTPS(class Globe& gb);
};