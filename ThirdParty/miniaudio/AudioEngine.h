#pragma once
#include "miniaudio\miniaudio.h"

class AudioEngine {
private:
	ma_engine engine;
public:
	AudioEngine();
	~AudioEngine();

	ma_engine& Get() noexcept { return engine; }
	const ma_engine& Get() const noexcept { return engine; }
};