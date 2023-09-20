#include "MIDI\Util.h"
#include <array>
#include <cassert>
#include <fstream>

namespace MIDI::util {

u32 beVLV_to_u32(u8* num) {
	u32 ret = 0;
	while ((*num & 0b1000'0000) != 0) {
		ret |= *num & 0b0111'1111;
		ret <<= 7;
		num++;
	}
	ret |= *num; // should already have msb 0, so masking isnt necessary
	return ret;
}

std::array<u8, 5> u32_to_beVLV(u32 num) {
	assert(num <= 0x0FFFFFFF);

	u32 val = 0; // stores the VLV in le
	std::array<u8, 5> ret; // ret[0..4] be VLV, ret[4] VLV length
	memset(ret.data(), 0, sizeof(ret)-1);
	ret[4] = 1;

	val |= num & 0b0111'1111;
	while ((num >>= 7) > 0) {
		val <<= 8;
		val |= 0b1000'0000 | (num & 0b0111'1111);
		ret[4]++;
	}

	// read le VLV into be VLV
	for (u8 i = 0; i < ret[4]; i++) {
		ret[i] = val & 0xff;
		val >>= 8;
	}

	return ret;
}

u32 read_beVLV(std::ifstream& file) {
	u8 chars[4] = {0,0,0,0};
	u8 i = 0;
	while (file.read(reinterpret_cast<char*>(chars+i), 1) && chars[i] & 0x80) i++;
	return beVLV_to_u32(chars);
}

u32 tempo_micros(u8* data) {
	union u24 {
		struct {
			u32 gbg : 8;
			u32 u : 24;
		};
		struct {
			u8 u8a;
			u8 bytes[3];
		};
	} val;
	val.gbg = 0;
	val.u = 0;
	val.bytes[2] = data[0];
	val.bytes[1] = data[1];
	val.bytes[0] = data[2];
	return val.u;
}

f32 tempo_bpm(u8* data) {
	return tempo_bpm(tempo_micros(data));
}

constexpr f32 tempo_bpm(u32 micros) {
	return 6e7f / micros;
}

}