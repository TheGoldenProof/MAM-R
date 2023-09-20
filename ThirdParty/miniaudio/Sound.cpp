#include "AudioEngine.h"
#include "Sound.h"
#include "Windows\framework.h"
#include <format>

Sound::Sound() {}

Sound::Sound(AudioEngine& engine, const wchar_t* file) {
	Open(engine, file);
}

Sound::~Sound() {
	//if (initialized) ma_sound_uninit(&sound);
}

Sound::operator bool() const {
	return initialized;
}

void Sound::Open(AudioEngine& engine, const wchar_t* file) {
	offset = 0;
	pauseFrame = 0;
	if (initialized) ma_sound_uninit(&sound);
	ma_result mr;
	u32 flags = MA_SOUND_FLAG_NO_PITCH | MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_STREAM;
	mr = ma_sound_init_from_file_w(&engine.Get(), file, flags, nullptr, nullptr, &sound);
	if (mr != MA_SUCCESS) {
		DEBUG_LOG(std::to_string(mr).c_str());
		initialized = false;
	} else {
		initialized = true;
		ma_sound_set_looping(&sound, false);
	}
}

void Sound::Play(AudioEngine& engine) {
	if (offset >= 0) {
		ma_sound_seek_to_pcm_frame(&sound, 0);
		ma_sound_set_start_time_in_pcm_frames(&sound, ma_engine_get_time_in_pcm_frames(&engine.Get()) + offset);
	} else if (offset < 0) {
		ma_sound_seek_to_pcm_frame(&sound, -offset);
		ma_sound_set_start_time_in_pcm_frames(&sound, ma_engine_get_time_in_pcm_frames(&engine.Get()));
	}
	ma_sound_start(&sound);
	isPaused = false;
}

void Sound::Stop() {
	ma_sound_stop(&sound);
	isPaused = false;
}

void Sound::Pause() {
	ma_sound_get_cursor_in_pcm_frames(&sound, &pauseFrame);
	ma_sound_stop(&sound);
	isPaused = true;
}

void Sound::Resume() {
	ma_sound_seek_to_pcm_frame(&sound, pauseFrame);
	ma_sound_start(&sound);
	isPaused = false;
}

void Sound::SetOffset(i32 offsetMillis) {
	ma_engine* engine = ma_sound_get_engine(&sound);
	i64 newOffset = offsetMillis * static_cast<i64>(ma_engine_get_sample_rate(engine)) / 1000;

	if (IsPlaying()) {
		i64 offsetDiff = newOffset - offset;
		u64 cursorPos = 0;
		ma_sound_get_cursor_in_pcm_frames(&sound, &cursorPos);
		if (offsetDiff < 0 && static_cast<u64>(-offsetDiff) > cursorPos) {
			cursorPos = 0;
		} else {
			cursorPos += offsetDiff;
		}
		ma_sound_seek_to_pcm_frame(&sound, cursorPos);
	}

	offset = newOffset;
}

void Sound::SetVolume(f32 volume) {
	if (initialized) ma_sound_set_volume(&sound, volume);
}

bool Sound::IsPlaying() const {
	return ma_sound_is_playing(&sound);
}

bool Sound::IsPaused() const {
	return isPaused;
}
