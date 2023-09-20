#pragma once
#include "miniaudio\miniaudio.h"
#include "TGLib\TGLib.h"

class Sound {
private:
	ma_sound sound{};
	i64 offset = 0;
	u64 pauseFrame = 0;
	bool initialized = false;
	bool isPaused = false;
public:
	Sound();
	Sound(class AudioEngine& engine, const wchar_t* file);
	~Sound();

	operator bool() const;

	void Open(class AudioEngine& engine, const wchar_t* file);

	void Play(class AudioEngine& engine);
	void Stop();
	void Pause();
	void Resume();

	void SetOffset(i32 offsetMillis);
	void SetVolume(f32 volume);

	bool IsPlaying() const;
	bool IsPaused() const;
};