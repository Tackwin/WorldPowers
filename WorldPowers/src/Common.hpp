#pragma once
#include <cstdint>
#include <random>

#define JCV_REAL_TYPE double
#define JCV_FABS fabs
#define JCV_ATAN2 atan2

using u08 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i08 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using dt_t = float;

namespace Common {

	extern u32 WINDOW_WIDTH;
	extern u32 WINDOW_HEIGHT;

	constexpr u64 SEED = 2;
	constexpr double PI = 3.141592653589793;

	constexpr double DEG_2_RAD = PI / 180.0;
	constexpr double RAD_2_DEG = 1 / DEG_2_RAD;

	template<typename T>
	constexpr inline u32 hash_combine_vector(u32 seed, const T* v, u32 n) {
		if (n == 0) return seed;

		std::hash<T> hasher;
		seed ^= hasher(*v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return hash_combine_vector(seed, ++v, --n);
	}

	constexpr inline u32 hash_combine(u32 A, u32 B) {
		return A + 0x9e3779b9 + (B << 6) + (B >> 2);
	}

	inline u64 xorshift96() {
		static u64 x = 123456789;
		static u64 y = 362436069;
		static u64 z = 521288629;

		x ^= x << 16;
		x ^= x >> 5;
		x ^= x << 1;
		auto t = x;
		x = y;
		y = z;
		z = t ^ x ^ y;

		return z;
	}

};

using namespace Common;

namespace std {
	template<typename T>
	struct hash<pair<T, T>> {
		constexpr size_t operator()(const std::pair<T, T>& p) const {
			auto A = std::template hash<T>()(p.first);
			auto B = std::template hash<T>()(p.second);
			return Common::hash_combine(A, B);
		}
	};
}

namespace xstd {
	template<typename T>
	constexpr T& other_one_pair(std::pair<T, T>& p, const T& a) noexcept {
		return p.first == a ? p.second : p.first;
	}
	template<typename T>
	constexpr const T& other_one_pair(const std::pair<T, T>& p, const T& a) noexcept {
		return p.first == a ? p.second : p.first;
	}
}