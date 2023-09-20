#pragma once
#include "MIDI\RawMidi.h"
#include <array>
#include <map>
#include <vector>

namespace MIDI {

struct Note {
	u64 startTick = 0;
	u64 lengthTicks = 0;
	u64 startMicro = 0;
	u64 lengthMicro = 0;
	u8 pitch = 64;
	u8 velocity = 100;
	u8 channel = 0;
};

struct Track {
	//std::array<std::vector<Note>, 16> notes;
	std::vector<Note> notes;
};

class CookedMidi {
private:
	HeaderChunk header;
	std::vector<Track> tracks;
	std::vector<std::pair<u32, u32>> tempoMap; // <tick, tempo micros>
public:
	CookedMidi();
	CookedMidi(const RawMidi& raw);

	void Cook(const RawMidi& raw);

	const HeaderChunk& GetHeader() const noexcept { return header; }
	const std::vector<Track>& GetTracks() const noexcept { return tracks; }
	const std::vector<std::pair<u32, u32>> GetTempoMap() const noexcept { return tempoMap; }
};

}