#pragma once

namespace Math {

enum INTERP_MODE {
	INTERP_MODE_LINEAR,
	INTERP_MODE_INSTANT,
	INTERP_MODE_DELAY,
};

template<typename T, typename F>
constexpr T interpolate_lin(const T& src, const T& dest, F alpha) {
	return static_cast<T>(src + (dest - src) * alpha);
}

template<typename T, typename F>
constexpr T interpolate_inst([[maybe_unused]] const T& src, const T& dest, [[maybe_unused]] F alpha) {
	return dest;
}

template<typename T, typename F>
constexpr T interpolate_delay(const T& src, const T& dest, F alpha) {
	return (alpha >= (F)1) ? src : dest;
}

template<typename T, typename F>
constexpr T interpolate(u32 mode, const T& src, const T& dest, F alpha) {
	if (mode == INTERP_MODE_INSTANT) {
		return interpolate_inst(src, dest, alpha);
	} else if (mode == INTERP_MODE_DELAY) {
		return interpolate_delay(src, dest, alpha);
	} else {
		return interpolate_lin(src, dest, alpha);
	}
}

}