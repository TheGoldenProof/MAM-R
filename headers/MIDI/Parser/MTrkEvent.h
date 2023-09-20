#pragma once
#include "TGLib\TGLib.h"

namespace MIDI {

struct MTrkEvent {
	u32 delta = 0;

	static std::unique_ptr<MTrkEvent> Make(std::ifstream& file);

	virtual ~MTrkEvent() {}

	virtual u8 GetEventType() const noexcept = 0;
	#ifndef DISTR
	virtual void DebugPrint(std::ostream& out) const = 0;
	#endif // !DISTR
private:
	static u8 lastMsg;
};

#include "MidiEvent.inl"

struct SysexEvent : public MTrkEvent {
	u8 f = 0;
	u32 length = 0;
	u8* data = nullptr;

	SysexEvent(u8 first_byte, std::ifstream& file);
	virtual ~SysexEvent() { if (data) delete data; }

	virtual u8 GetEventType() const noexcept override { return f; }

	#ifndef DISTR
	virtual void DebugPrint(std::ostream& out) const override;
	#endif // !DISTR
};

struct MetaEvent : public MTrkEvent {
	u8 ff = 0;
	u8 type = 0;
	u32 length = 0;
	u8* data = nullptr;

	MetaEvent(std::ifstream& file);
	virtual ~MetaEvent() { if (data) delete data; }

	virtual u8 GetEventType() const noexcept override { return ff; }

	#ifndef DISTR
	virtual void DebugPrint(std::ostream& out) const override;
	#endif // !DISTR
};

}