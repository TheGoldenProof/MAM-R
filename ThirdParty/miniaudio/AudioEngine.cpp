#include "miniaudio\AudioEngine.h"
#include "TGLib\TGLib.h"
#include "Windows\framework.h"

AudioEngine::AudioEngine() {
	ma_result mr;
	mr = ma_engine_init(nullptr, &engine);
	if (mr != MA_SUCCESS) {
		DEBUG_LOG(std::to_string(static_cast<i32>(mr)).c_str());
	}
}

AudioEngine::~AudioEngine() {
	ma_engine_uninit(&engine);
}
