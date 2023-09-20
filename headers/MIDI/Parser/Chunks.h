#pragma once
#include "MIDI\Parser\MTrkEvent.h"
#include "TGLib\TGLib.h"
#include <vector>

namespace MIDI {

struct Chunk {
	char type[4] = {0, 0, 0, 0};
	u32 length = 0;

	Chunk() {}
	Chunk(const char type[4], u32 length);
};

struct HeaderChunk : public Chunk {
	u16 format = 0;
	u16 ntrks = 0;
	union div {
		u16 u = 0;
		union {
			struct {
				u16 ticksPerQuarter : 15;
				u16 fmt : 1;
			};
			struct {
				u8 ticksPerFrame;
				i8 smpte;
			};
		};
	} division;

	HeaderChunk();
	HeaderChunk(std::ifstream& file);

	#ifndef DISTR
	void DebugPrint(std::ostream& out) const;
	#endif // !DISTR

};

struct TrackChunk : public Chunk {
	std::vector<std::unique_ptr<MTrkEvent>> events;

	TrackChunk();
	TrackChunk(std::ifstream& file);

	#ifndef DISTR
	void DebugPrint(std::ostream& out) const;
	#endif // !DISTR
};

}