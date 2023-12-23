#include "MIDI\Enums.h"
#include "MIDI\Parser\MTrkEvent.h"
#include "MIDI\Util.h"
#include <format>
#include <fstream>

namespace MIDI {

#ifndef DISTR
/*void NoteEvent::DebugPrint(std::ostream& out) const {
	out << "        midi  ";

	switch (msg.status) {
	case MIDI_EVENT_STATUS_NOTE_OFF:

	case MIDI_EVENT_STATUS_NOTE_ON:

	case MIDI_EVENT_STATUS_POLY_KEY_PRESSURE:

	case MIDI_EVENT_STATUS_CTRL_CHANGE:

	case MIDI_EVENT_STATUS_PRGM_CHANGE:

	case MIDI_EVENT_STATUS_CHANNEL_PRESSURE:

	case MIDI_EVENT_STATUS_PITCH_WHEEL:

	case MIDI_EVENT_STATUS_SYSTEM:
		switch (msg.channel) {
		case MIDI_EVENT_SYSCH_SONG_POS:

		case MIDI_EVENT_SYSCH_SONG_SELECT:

		case MIDI_EVENT_SYSCH_TUNE:
		case MIDI_EVENT_SYSCH_TIMING:
		case MIDI_EVENT_SYSCH_START:
		case MIDI_EVENT_SYSCH_RESUME:
		case MIDI_EVENT_SYSCH_STOP:
		case MIDI_EVENT_SYSCH_SENSING:

		}
	}

	out << "\n";
}*/

void MidiEvent::DebugPrint(std::ostream& out) const {
	out << "        " << std::format("{:<6d}", delta) << "  midi  ";

	if (msg.status() == MIDI_EVENT_STATUS_SYSTEM) {
		switch (msg.channel()) {
		case MIDI_EVENT_SYSCH_TUNE:
			out << "tune request";
			break;
		case MIDI_EVENT_SYSCH_TIMING:
			out << "timing clock";
			break;
		case MIDI_EVENT_SYSCH_START:
			out << "seq start";
			break;
		case MIDI_EVENT_SYSCH_RESUME:
			out << "seq resume";
			break;
		case MIDI_EVENT_SYSCH_STOP:
			out << "seq stop";
			break;
		case MIDI_EVENT_SYSCH_SENSING:
			out << "active sensing";
			break;
		default:
			out << "unknown dataless system event";
			break;
		}
	} else {
		out << "unknown dataless midi event";
	}

	out << "\n";
}

void MidiEventOneByte::DebugPrint(std::ostream& out) const {
	out << "        " << std::format("{:<6d}", delta) << "  midi  ";

	switch (msg.status()) {
	case MIDI_EVENT_STATUS_PRGM_CHANGE:
		out << std::format("ch:{:d}", msg.channel()) << "  prgm:" << static_cast<u32>(data1);
		break;
	case MIDI_EVENT_STATUS_CHANNEL_PRESSURE:
		out << std::format("ch:{:d}", msg.channel()) << "  ch pressure:" << static_cast<u32>(data1);
		break;
	case MIDI_EVENT_STATUS_SYSTEM:
		if (msg.channel() == MIDI_EVENT_SYSCH_SONG_SELECT) {
			out << "song sel:" << static_cast<u32>(data1);
		} else {
			out << "unknown 1byte system event  data:" << static_cast<u32>(data1);
		}
		break;
	default:
		out << "unknown 1byte midi event  data:" << static_cast<u32>(data1);
		break;
	}

	out << "\n";
}

void MidiEventTwoBytes::DebugPrint(std::ostream& out) const {
	out << "        " << std::format("{:<6d}", delta) << "  midi  ";

	switch (msg.status()) {
	case MIDI_EVENT_STATUS_NOTE_OFF:
		out << std::format("ch:{:d}", msg.channel()) << "  note off  key:" << static_cast<u32>(data1) << "  vel:" << static_cast<u32>(data2);
		break;
	case MIDI_EVENT_STATUS_NOTE_ON:
		out << std::format("ch:{:d}", msg.channel()) << "  note on  key:" << static_cast<u32>(data1) << "  vel:" << static_cast<u32>(data2);
		break;
	case MIDI_EVENT_STATUS_POLY_KEY_PRESSURE:
		out << std::format("ch:{:d}", msg.channel()) << "  pressure  key:" << static_cast<u32>(data1) << "  vel:" << static_cast<u32>(data2);
		break;
	case MIDI_EVENT_STATUS_PITCH_WHEEL:
		out << std::format("ch:{:d}", msg.channel()) << "  pitchwheel:" << ((static_cast<u32>(data2) << 7) + static_cast<u32>(data1));
		break;
	case MIDI_EVENT_STATUS_SYSTEM:
		if (msg.channel() == MIDI_EVENT_SYSCH_SONG_POS) {
			out << "song pos:" << ((static_cast<u32>(data2) << 7) + static_cast<u32>(data1));
		} else {
			out << "unknown 2byte system event  data: " << static_cast<u32>(data1) << " " << static_cast<u32>(data2);
		}
		break;
	default:
		out << "unknown 2byte midi event  data: " << static_cast<u32>(data1) << " " << static_cast<u32>(data2);
		break;
	}

	out << "\n";
}

void ControlEvent::DebugPrint(std::ostream& out) const {
	out << "        " << std::format("{:<6d}", delta) << "  midi  ";
	out << std::format("ch:{:d}", msg.channel()) << "  ctrl change  ctrl:" << static_cast<u32>(data1) << "  val:" << static_cast<u32>(data2);
	//TODO: print control name instead of id
	out << "\n";
}
#endif // !DISTR

}
