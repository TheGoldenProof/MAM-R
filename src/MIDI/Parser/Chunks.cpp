#include "MIDI\Parser\Chunks.h"
#include "MIDI\Enums.h"
#include "MIDI\Util.h"
#include <cassert>
#include <fstream>

#ifndef DISTR
#include <format>
#endif // !DISTR


#include "Windows\framework.h"

static bool checkStream(std::ifstream& file) {
	return file.good();
	/*std::string str("000");
	auto state = file.rdstate();
	if (state & std::ios::badbit) str[1] = '1';
	if (state & std::ios::failbit) str[2] = '1';
	if (state & std::ios::eofbit) str[3] = '1';
	DEBUG_LOG(("Stream state: " + str + "\n").c_str());
	return !strncmp(str.c_str(), "000", 3);*/
}

MIDI::Chunk::Chunk(const char type_[4], u32 length) : length(length) {
	type[0] = type_[0];
	type[1] = type_[1];
	type[2] = type_[2];
	type[3] = type_[3];
}

MIDI::HeaderChunk::HeaderChunk() : Chunk("MThd", 6) {}

#define READ(var) file.read(reinterpret_cast<char*>(&var), sizeof(var)); var = util::bele(var); if (!checkStream(file)) return

MIDI::HeaderChunk::HeaderChunk(std::ifstream& file) : HeaderChunk() {
	if (!checkStream(file)) return;
	READ(length);
	auto pos = file.tellg();
	READ(format);
	READ(ntrks);
	READ(division.u);
	file.seekg(pos + static_cast<std::streamoff>(length));

	if (!(format == MIDI_FORMAT_0 || format == MIDI_FORMAT_1)) {
		DEBUG_LOG(std::format("Unsupported MIDI format: {:d}\n", format).c_str());
	}
}

#ifndef DISTR
void MIDI::HeaderChunk::DebugPrint(std::ostream& out) const {
	out << std::format("{:s}  len:{:d}  format:{:d} ntrks:{:d}  ", std::string(type, sizeof(type)), length, format, ntrks);
	if (division.fmt == MIDI_DIVISION_FORMAT_TPQN) {
		out << std::format("ticks/qtr:{:d}\n", division.ticksPerQuarter);
	} else {
		out << std::format("fps:{:d}  ticks/frame:{:d}\n", -division.smpte, division.ticksPerFrame);
	}
}

MIDI::TrackChunk::TrackChunk() : Chunk("MTrk", 0) {}
#endif

MIDI::TrackChunk::TrackChunk(std::ifstream& file) : TrackChunk() {
	if (!checkStream(file)) return;
	READ(length);
	auto pos = file.tellg();
	bool eot = false;
	while (checkStream(file) && !eot) {
		auto pEvent = MTrkEvent::Make(file);
		//assert(pEvent);
		if (!pEvent) {
			DEBUG_LOG("pEvent was nullptr.\n");
			continue;
		}
		auto pME = reinterpret_cast<MetaEvent*>(pEvent.get()); 
		eot = (pME->ff == 0xff && pME->type == MIDI_META_EVENT_EOT);
		events.emplace_back(std::move(pEvent));
	}
	file.seekg(pos + static_cast<std::streamoff>(length));
}

#ifndef DISTR
void MIDI::TrackChunk::DebugPrint(std::ostream& out) const {
	out << std::format("{:s}  len:{:d}\n", std::string(type, sizeof(type)), length);

	for (const auto& pEvent : events) {
		pEvent->DebugPrint(out);
	}

	out.flush();
}
#endif
