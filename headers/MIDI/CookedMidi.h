#pragma once
#include "MIDI\RawMidi.h"
#include <chrono>
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
	std::vector<Note> notes;
};

class CookedMidi {
public:
	class TempoMap {
		friend CookedMidi;
	public:
		TempoMap();
		std::map<i64, u32> _map; // <time micros, tempo micros>
		std::pair<i64, u32> operator[](i64 timeMicros) const;
		std::map<i64, u32>::const_iterator GetIt(i64 timeMicros) const;
		std::pair<i64, u32> next(i64 timeMicros) const;
	};
private:
	HeaderChunk header;
	std::vector<Track> tracks;
	TempoMap tempoMap;
public:
	CookedMidi();
	CookedMidi(const RawMidi& raw);

	void Cook(const RawMidi& raw);

	const HeaderChunk& GetHeader() const noexcept { return header; }
	const std::vector<Track>& GetTracks() const noexcept { return tracks; }
	const TempoMap& GetTempoMap() const noexcept { return tempoMap; }
private:
	void CookTempoMap(const RawMidi& raw);
	//void CookNotes(const RawMidi& raw);
};

}