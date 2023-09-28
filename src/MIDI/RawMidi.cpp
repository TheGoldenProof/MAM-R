#include "Globe.h"
#include "MIDI\Enums.h"
#include "MIDI\Util.h"
#include "MIDI\RawMidi.h"
#include "Windows\Window.h"
#include <fstream>

namespace MIDI {
RawMidi::RawMidi() {}

RawMidi::RawMidi(Globe& gb, const std::wstring& path) {
	Open(gb, path);
}

void RawMidi::Open(Globe& gb, const std::wstring& path) {
	tracks.clear();
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) {
		MessageBox(gb.Wnd().GetHWND(), L"Couldn't open file.", nullptr, MB_OK | MB_ICONERROR);
		return;
	}

	union un {
		u32 u;
		char bytes[4];
	} chunk;
	bool readHeader = false;

	while (file.good()) {
		file.read(chunk.bytes, 4);
		if (!file.good()) break;
		switch (util::bele(chunk.u)) {
		case MIDI_CHUNK_TYPE_HEADER:
			if (readHeader) {
				DEBUG_LOG("Multiple header chunks encountered\n");
				break;
			}
			header = HeaderChunk(file);
			readHeader = true;
			break;
		case MIDI_CHUNK_TYPE_TRACK:
			tracks.emplace_back(TrackChunk(file));
			break;
		default:
			DEBUG_LOG(("Unknown chunk type: " + std::string(chunk.bytes, 4) + "\n").c_str());
		}
	}
	if (file.fail()) {
		DEBUG_LOG("Something might have gone wrong reading the file.\n");
	}
}

#ifndef DISTR
void RawMidi::DebugPrint(std::ostream& out) const {
	out << "--------------------- MIDI debug dump ---------------------" << std::endl;
	header.DebugPrint(out);
	for (const auto& track : tracks) {
		track.DebugPrint(out);
	}
	out << "-----------------------------------------------------------" << std::endl;
}
#endif

}
