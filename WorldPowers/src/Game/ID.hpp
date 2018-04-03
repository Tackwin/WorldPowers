#pragma once

#include <chrono>
#include "./../Common.hpp"

struct ID {
private:
	using type_t = u64;

	friend std::hash<ID>;
public:
	type_t _id{ 0 };

	constexpr type_t operator()() const noexcept {
		return _id;
	}

	constexpr bool operator==(const ID& B) const noexcept {
		return _id == B();
	}
	constexpr bool operator!=(const ID& B) const noexcept {
		return !(*this == B);
	}

	constexpr bool operator<(const ID& B) const noexcept {
		return _id < B();
	}

	constexpr explicit operator bool() const noexcept {
		return _id != NILL();
	}

	static ID construct() {
		i64 ns = std::chrono::steady_clock::now().time_since_epoch().count();
		i64 rand = (i64)Common::xorshift96();

		type_t i = (type_t)(
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
			return std::hash<ID::type_t>()(id());
		}
	};
}