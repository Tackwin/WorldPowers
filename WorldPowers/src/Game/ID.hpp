#pragma once

#include <chrono>
#include "./../Common.hpp"

struct ID {
	u64 _id{ 0 };

	constexpr u64 operator()() const {
		return _id;
	}

	constexpr bool operator==(const ID& B) const {
		return _id == B();
	}
	constexpr bool operator!=(const ID& B) const {
		return !(*this == B);
	}

	constexpr bool operator<(const ID& B) const {
		return _id < B();
	}

	constexpr explicit operator bool() const {
		return _id != NILL();
	}

	static ID construct() {
		i64 ns = std::chrono::steady_clock::now().time_since_epoch().count();
		i64 rand = (i64)Common::xorshift96();

		u64 i = (u64)(
					(ns				& 0x0000'00FF'FFFF'FFFF) |
					((rand << 32)	& 0xFFFF'FF00'0000'0000)
				);

		return ID{ i + 1 };
	}

	static const ID NILL;
};

namespace std {
	template<>
	struct hash<ID> {
		constexpr size_t operator()(const ID& id) const {
			return std::hash<u64>()(id());
		}
	};
}