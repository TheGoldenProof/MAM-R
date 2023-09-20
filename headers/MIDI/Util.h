#pragma once
#include "TGLib\TGLib.h"
#include <intrin.h>

namespace MIDI::util {

template <typename T> requires(std::is_integral_v<T>)
constexpr T bele(T val) {
	if constexpr (sizeof(T) == 1) { return val; } else {
		union Bytes {
			T t;
			u8 bytes[sizeof(T)];
		} src, dest;
		src.t = val;
		dest.t = 0;
	
		for (u8 i = 0; i < sizeof(T); i++)
			dest.bytes[i] = src.bytes[sizeof(T) - i - 1];

		return dest.t;
	}

	
}

u32 beVLV_to_u32(u8* num);

std::array<u8, 5> u32_to_beVLV(u32 num); // last index is the length of the VLV. starts filling from [0].

u32 read_beVLV(std::ifstream& file);

u32 tempo_micros(u8* data);
f32 tempo_bpm(u8* data);
constexpr f32 tempo_bpm(u32 micros);

}