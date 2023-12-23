#include "MIDI\Enums.h"
#include "MIDI\Parser\MTrkEvent.h"
#include "MIDI\Util.h"
#include <format>
#include <fstream>

namespace MIDI {

#define READ(var) file.read(reinterpret_cast<char*>(&var), sizeof(var)); var = util::bele(var); CHECK_FILE()
#define READPN(var, byte_count) file.read(reinterpret_cast<char*>(var), byte_count); CHECK_FILE()

MetaEvent::MetaEvent(std::ifstream& file) {
#define CHECK_FILE() if (!file.good()) return
	ff = 0xFF;
	READ(type);
	length = util::read_beVLV(file);
	CHECK_FILE();

	auto pos = file.tellg();
	if (length != 0) {
		data = new u8[length];
		READPN(data, length);
		file.seekg(pos + static_cast<std::streamoff>(length));
	}
#undef CHECK_FILE
}

#ifndef DISTR
void MetaEvent::DebugPrint(std::ostream& out) const {
	out << "        " << std::format("{:<6d}", delta) << "  meta  " << std::format("type:{:x}  len:{:d}  ", type, length);
	switch (type) {
	case MIDI_META_EVENT_SEQUENCE_NUMBER:
		out << "sqn#:" << util::bele(*reinterpret_cast<u16*>(data));
		break;
	case MIDI_META_EVENT_TEXT:
		out << "text:\"" << std::string(reinterpret_cast<char*>(data), length) << "\"";
		break;
	case MIDI_META_EVENT_COPYRIGHT:
		out << "cpyr:\"" << std::string(reinterpret_cast<char*>(data), length) << "\"";
		break;
	case MIDI_META_EVENT_TRACK_NAME:
		out << "track:\"" << std::string(reinterpret_cast<char*>(data), length) << "\"";
		break;
	case MIDI_META_EVENT_INSTR_NAME:
		out << "instr:\"" << std::string(reinterpret_cast<char*>(data), length) << "\"";
		break;
	case MIDI_META_EVENT_LYRIC:
		out << "lyric:\"" << std::string(reinterpret_cast<char*>(data), length) << "\"";
		break;
	case MIDI_META_EVENT_MARKER:
		out << "mark:\"" << std::string(reinterpret_cast<char*>(data), length) << "\"";
		break;
	case MIDI_META_EVENT_CUE:
		out << "cue:\"" << std::string(reinterpret_cast<char*>(data), length) << "\"";
		break;
	case MIDI_META_EVENT_CHANNEL:
		out << "chn:" << *data;
		break;
	case MIDI_META_EVENT_EOT:
		out << "end";
		break;
	case MIDI_META_EVENT_TEMPO: {
		const u32 micros = util::tempo_micros(data);
		const f32 tempo = util::tempo_bpm(micros);

		out << "tempo:" << micros << " (" << tempo << "bpm)";
		break;
	}
	case MIDI_META_EVENT_SMPTE_OFFSET:
		out << "offset: " << std::format("{:d}h {:d}m {:d}s {:d}fr {:d}ff", data[0], data[1], data[2], data[3], data[4]);
		break;
	case MIDI_META_EVENT_TIME_SIG:
		out << "tsig:  " << std::format("{:d}/{:d} metr:{:d} 32nds:{:d}", data[0], 1 << data[1], data[2], data[3]);
		break;
	case MIDI_META_EVENT_KEY_SIG:
		out << "ksig:" << static_cast<i32>(static_cast<i8>(data[0])) << (data[1] ? "m" : "M");
		break;
	case MIDI_META_EVENT_SEQ_SPEC:
		out << "custom:";
		for (u32 i = 0; i < length; i++) out << std::format(" {:x}", data[i]);
		break;
	}

	out << "\n";
}
#endif

}