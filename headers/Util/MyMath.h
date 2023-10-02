#pragma once
#include "TGLib/TGLib.h"
#include <DirectXMath.h>
#include <cmath>
#include <type_traits>

namespace {
#include "MyMathLookups.inl"
}

namespace Math {

template <typename T> constexpr T PI = T(3.1415926535897932384626433832795L);
template <typename T> constexpr T TWO_PI = T(6.283185307179586476925286766559L);
template <typename T> constexpr T HALF_PI = T(1.5707963267948966192313216916398L);
template <typename T> constexpr T QTR_PI = T(0.78539816339744830961566084581988L);

template<typename T>
constexpr T sq(const T& x) {
	return x * x;
}

template<typename T>
constexpr T map(const T& value, const T& fromLow, const T& fromHigh, const T& toLow, const T& toHigh) {
	const T inScale = (fromHigh - fromLow); 
	if (inScale == 0) return (toLow + toHigh) / 2;
	const T outScale = (toHigh - toLow);
	return (value - fromLow) * outScale / inScale + toLow;
}

template<typename T>
T wrapAngle_rad(const T& theta) {
	constexpr T pi = PI<T>;
	constexpr T twoPi = TWO_PI<T>;
	const T mod = fmod(theta, twoPi);

	if (mod > pi) {
		return mod - twoPi;
	} else if (mod < -pi) {
		return mod + twoPi;
	} else return mod;
}
template<typename T>
T wrapAngle_deg(const T& theta) {
	const T mod = fmod(theta, static_cast<T>(360));

	if (mod > static_cast<T>(180)) {
		return mod - static_cast<T>(360);
	} else if (mod < static_cast<T>(-180)) {
		return mod + static_cast<T>(360);
	} else return mod;
}
template<typename T> requires (std::is_integral_v<T>)
constexpr T wrapAngle_deg_int(T theta) {
	T mod = theta % static_cast<T>(360);
	if (mod < 0) mod += static_cast<T>(360);
	return mod;
}

template<typename T> constexpr T to_rad(T deg) { return deg * (PI<T>/180); }
template<typename T> constexpr T to_deg(T rad) { return rad * (180/PI<T>); }

template<typename T> T sin_deg(T deg) { return sin(to_rad(deg)); }
template<typename T> T cos_deg(T deg) { return cos(to_rad(deg)); }
template<typename T> T tan_deg(T deg) { return tan(to_rad(deg)); }
template<typename T> T csc_deg(T deg) { return 1/sin(to_rad(deg)); }
template<typename T> T sec_deg(T deg) { return 1/cos(to_rad(deg)); }
template<typename T> T cot_deg(T deg) { return 1/tan(to_rad(deg)); }

template<typename T1, typename T2>
constexpr T1 fastsin_deg(T2 deg) {
	const usize i = wrapAngle_deg_int(static_cast<i64>(deg));
	return static_cast<T1>(SIN_LOOKUP[i]);
}
template<typename T1, typename T2>
constexpr T1 fastcos_deg(T2 deg) {
	const usize i = wrapAngle_deg_int(static_cast<i64>(deg)+90);
	return static_cast<T1>(SIN_LOOKUP[i]);
}
template<typename T1, typename T2>
constexpr T1 fasttan_deg(T2 deg) {
	const usize i = wrapAngle_deg_int(static_cast<i64>(deg));
	return static_cast<T1>(TAN_LOOKUP[i]);
}
template<typename T1, typename T2>
constexpr T1 fastcsc_deg(T2 deg) {
	const usize i = wrapAngle_deg_int(static_cast<i64>(deg));
	return static_cast<T1>(CSC_LOOKUP[i]);
}
template<typename T1, typename T2>
constexpr T1 fastsec_deg(T2 deg) {
	const usize i = wrapAngle_deg_int(static_cast<i64>(deg)+90);
	return static_cast<T1>(CSC_LOOKUP[i]);
}
template<typename T1, typename T2>
constexpr T1 fastcot_deg(T2 deg) {
	const usize i = wrapAngle_deg_int(static_cast<i64>(deg));
	return static_cast<T1>(COT_LOOKUP[i]);
}

template<typename T>
constexpr T fastatan_rad(T v) requires (std::is_floating_point_v<T>) {
	const T a1  = static_cast<T>( 0.99997726);
	const T a3  = static_cast<T>(-0.33262347);
	const T a5  = static_cast<T>( 0.19354346);
	const T a7  = static_cast<T>(-0.11643287);
	const T a9  = static_cast<T>( 0.05265332);
	const T a11 = static_cast<T>(-0.01172120);

	const T v2 = v * v;
	return v * fma(v2, fma(v2, fma(v2, fma(v2, fma(v2, a11, a9), a7), a5), a3), a1);
}
template<typename T>
constexpr T fastatan_deg(T v) requires (std::is_floating_point_v<T>) {
	return to_deg(fastatan_rad(v));
}
template<typename T>
constexpr T fastatan2_rad(T y, T x) requires (std::is_floating_point_v<T>) {
	const bool swap = fabs(x) < fabs(y);
	const T input = (swap ? x : y) / (swap ? y : x);
	T res = fastatan_rad(input);
	res = swap ? copysign(static_cast<T>(HALF_PI), input) - res : res;
	if (x < 0) res = copysign(static_cast<T>(PI), y) + res;
	return res;
}
template<typename T>
constexpr T fastatan2_deg(T y, T x) requires (std::is_floating_point_v<T>) {
	return to_deg(fastatan2_rad(y, x));
}

}

#include "Interp.h"
