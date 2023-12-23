#include "MIDI\Enums.h"
#include "MIDI\Parser\MTrkEvent.h"
#include "MIDI\Util.h"
#include <format>
#include <fstream>

namespace MIDI {

#define READPN(var, byte_count) file.read(reinterpret_cast<char*>(var), byte_count); CHECK_FILE()

#define CHECK_FILE() if (!file.good()) return
SysexEvent::SysexEvent(u8 first_byte, std::ifstream& file) {
	f = first_byte;
	length = util::read_beVLV(file);
	CHECK_FILE();

	auto pos = file.tellg();
	data = new u8[length];
	READPN(data, length);
	file.seekg(pos + static_cast<std::streamoff>(length));
}
#undef CHECK_FILE

#ifndef DISTR
void MIDI::SysexEvent::DebugPrint(std::ostream& out) const {
	out << "        " << std::format("{:<6d}", delta) << "  sysex  " << std::format("type:{:x}  len:{:d}  ", f, length) << "  data: ";
	for (u32 i = 0; i < length; i++) out << std::format(" {:x}", data[i]);
	out << "\n";
}
#endif

}