#pragma once
#include "MIDI\Parser\Chunks.h"
#include "TGLib\TGLib.h"

namespace MIDI {

class RawMidi {
public:
	HeaderChunk header;
	std::vector<TrackChunk> tracks;
public:
	RawMidi(class Globe& gb, const std::wstring& path);

	#ifndef DISTR
	void DebugPrint(std::ostream& out) const;
	#endif // !DISTR

};

}