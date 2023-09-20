#include "MTrkEvent.h"

struct MidiEvent : public ::MIDI::MTrkEvent {
	struct message {
		u8 v;
		u8 status() const { return (v & 0xf0) >> 4; }
		u8 channel() const { return v & 0x0f; }
	} msg;
	
	MidiEvent() { msg.v = 0; }
	virtual ~MidiEvent() {}

	virtual u8 GetEventType() const noexcept override { return msg.v; }
	u8 Status() const { return msg.status(); }
	u8 Channel() const { return msg.channel(); }

	#ifndef DISTR
	virtual void DebugPrint(std::ostream& out) const override;
	#endif // !DISTR
};

struct MidiEventOneByte : public MidiEvent {
	u8 data1 = 0;

	virtual ~MidiEventOneByte() {}
	#ifndef DISTR
	virtual void DebugPrint(std::ostream& out) const override;
	#endif // !DISTR
};

struct MidiEventTwoBytes : public MidiEvent {
	u8 data1 = 0;
	u8 data2 = 0;

	virtual ~MidiEventTwoBytes() {}
	#ifndef DISTR
	virtual void DebugPrint(std::ostream& out) const override;
	#endif // !DISTR
};

struct NoteEvent : public MidiEventTwoBytes {
	u8& key() noexcept { return data1; }
	const u8& key() const noexcept { return data1; }
	u8& vel() noexcept { return data2; }
	const u8& vel() const noexcept { return data2; }
};

struct ControlEvent : public MidiEventTwoBytes {
	u8& control() noexcept { return data1; }
	u8& val() noexcept { return data2; }

	#ifndef DISTR
	virtual void DebugPrint(std::ostream& out) const override;
	#endif // !DISTR
};

struct ProgramChangeEvent : public MidiEventOneByte {
	u8& patch() noexcept { return data1; }
};

struct ChannelPressureEvent : public MidiEventOneByte {
	u8& vel() noexcept { return data1; }
};

struct PitchWheelEvent : public MidiEventTwoBytes {
	u16 pitch() const noexcept { return (data2 << 7) + data1; }
};

struct SongPositionEvent : public MidiEventTwoBytes {
	u16 pos() const noexcept { return (data2 << 7) + data1; }
};

struct SongSelectEvent : public MidiEventOneByte {
	u8& song() noexcept { return data1; }
};
