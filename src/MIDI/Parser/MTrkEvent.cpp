#include "MIDI\Enums.h"
#include "MIDI\Parser\MTrkEvent.h"
#include "MIDI\Util.h"
#include <fstream>

#include <cassert>
#include "Windows\framework.h"

static bool checkStream(std::ifstream& file) {
	return file.good();
	/*std::string str("000");
	auto state = file.rdstate();
	if (state & std::ios::badbit) str[1] = '1';
	if (state & std::ios::failbit) str[2] = '1';
	if (state & std::ios::eofbit) str[3] = '1';
	bool ret = !strncmp(str.c_str(), "000", 3);
	if (!ret) DEBUG_LOG(("Stream state: " + str + "\n").c_str());
	return ret;*/
}

namespace MIDI {

u8 MTrkEvent::lastMsg = 0;

#define READ(var) file.read(reinterpret_cast<char*>(&var), sizeof(var)); var = util::bele(var); CHECK_FILE()

std::unique_ptr<MTrkEvent> MTrkEvent::Make(std::ifstream& file) {
	#define CHECK_FILE() if (!checkStream(file)) return {nullptr}
	u32 delta = util::read_beVLV(file);
	u8 mtype; READ(mtype);
	
	if (mtype == 0xFF) { // MIDI_EVENT_STATUS_SYSTEM << 4 + MIDI_EVENT_SYSCH_RESET; NOTE: in a MIDI file, this is interpreted as a meta event, but from a port its interpreted as a reset.
		auto pEvent = std::make_unique<MetaEvent>(file);
		pEvent->delta = delta;
		return std::move(pEvent);
	} else if (mtype == 0xF0 || mtype == 0xF7) { // MIDI_EVENT_STATUS_SYSTEM << 4 + MIDI_EVENT_SYSCH_(SYSTEM_EXCLUSIVE or EXCLUSIVE_END)
		auto pEvent = std::make_unique<SysexEvent>(mtype, file);
		pEvent->delta = delta;
		return std::move(pEvent);
	} else {
		MidiEvent::message msg{ 0 };
		msg.v = mtype;

		if (!(msg.status() & 0x8)) {
			msg.v = lastMsg;
			file.seekg(static_cast<std::streamoff>(-1), std::ios::cur);
		}
		lastMsg = msg.v;

		#define EVENT(Type) auto pEvent = std::make_unique<Type>(); pEvent->msg = msg; pEvent->delta = delta
		#define MAKE0(Type) { EVENT(Type); return pEvent; }
		#define MAKE1(Type) { EVENT(Type); READ(pEvent->data1); return pEvent; }
		#define MAKE2(Type) { EVENT(Type); READ(pEvent->data1); READ(pEvent->data2); return pEvent; }
		switch (msg.status()) {
		case MIDI_EVENT_STATUS_NOTE_OFF: 
		case MIDI_EVENT_STATUS_NOTE_ON: 
		case MIDI_EVENT_STATUS_POLY_KEY_PRESSURE:
			MAKE2(NoteEvent);
		case MIDI_EVENT_STATUS_CTRL_CHANGE:
			MAKE2(ControlEvent);
		case MIDI_EVENT_STATUS_PRGM_CHANGE:
			MAKE1(ProgramChangeEvent);
		case MIDI_EVENT_STATUS_CHANNEL_PRESSURE:
			MAKE1(ChannelPressureEvent);
		case MIDI_EVENT_STATUS_PITCH_WHEEL:
			MAKE2(PitchWheelEvent);
		case MIDI_EVENT_STATUS_SYSTEM:
			switch (msg.channel()) {
			case MIDI_EVENT_SYSCH_SONG_POS:
				MAKE2(SongPositionEvent);
			case MIDI_EVENT_SYSCH_SONG_SELECT:
				MAKE1(SongSelectEvent);
			case MIDI_EVENT_SYSCH_TUNE:
			case MIDI_EVENT_SYSCH_TIMING:
			case MIDI_EVENT_SYSCH_START:
			case MIDI_EVENT_SYSCH_RESUME:
			case MIDI_EVENT_SYSCH_STOP:
			case MIDI_EVENT_SYSCH_SENSING:
				MAKE0(MidiEvent);
			}
		}
		#undef EVENT
		#undef MAKE0
		#undef MAKE1
		#undef MAKE2

		DEBUG_LOG(std::format("unknown midi message: {:d} ({:b})\n", msg.v, msg.v).c_str());
		//assert(false);
		return { nullptr };
	}
	#undef CHECK_FILE
}

}