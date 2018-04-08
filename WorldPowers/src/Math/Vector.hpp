#pragma once

#include <type_traits>
#include "Common.hpp"
#pragma warning(push)
#pragma warning(disable: 4201)

#define COLOR_UNROLL(x) (x).r, (x).g, (x).b, (x).a
#define XYZW_UNROLL(v) (v).x, (v).y, (v).z, (v).w

template<size_t D, typename T>
struct __vec_member {
	T components[D];
};
template<typename T>
struct __vec_member<1, T> {
	union {
		struct {
			T x;
		};
		T components[1];
	};

	__vec_member() : x(0) {}
	__vec_member(T x) : x(x) {}
};
template<typename T>
struct __vec_member<2, T> {
	union {
		struct {
			T x;
			T y;
		};
		T components[2];
	};

	__vec_member() : x(0), y(0) {}
	__vec_member(T x, T y) : x(x), y(y) {}
};
template<typename T>
struct __vec_member<3, T> {
	union {
		struct {
			T x;
			T y;
			T z;
		};
		T components[3];
	};

	__vec_member() : x(0), y(0), z(0) {}
	__vec_member(T x, T y, T z) : x(x), y(y), z(z) {}
};
template<typename T>
struct __vec_member<4, T> {
	union {
		struct {
			T x;
			T y;
			T z;
			T w;
		};
		struct {
			T r;
			T g;
			T b;
			T a;
		};
		T components[4];
	};

	__vec_member() : x(0), y(0), z(0), w(0) {}
	__vec_member(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
};


template<size_t D, typename T>
struct Vector;

template<typename T> using Vector2 = Vector<2U, T>;
template<typename T> using Vector3 = Vector<3U, T>;
template<typename T> using Vector4 = Vector<4U, T>;
using Vector2u = Vector2<u32>;
using Vector2i = Vector2<i32>;
using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector4u = Vector4<u32>;
using Vector4f = Vector4<float>;
using Vector4d = Vector4<double>;

template<size_t D, typename T = float>
struct Vector : public __vec_member<D, T> {

	static Vector<D, T> createUnitVector(float angles[D]) {
		Vector<D, T> result;
		result[0] = cosf(angles[0]);
		for (size_t i = 1u; i < D; ++i) {

			result[i] = (i + 1u != D) ?
				cosf(angles[i]) :
				1;

			for (size_t j = 0u; j < D - 1u; ++j) {
				result[i] *= sinf(angles[j]);
			}
		}
		return result;
	}
	static Vector<2, T> createUnitVector(float angles) { // i'm not doing all the shit above for 2d
		return { cosf(angles), sinf(angles) };
	}
	static Vector<D, T> createUnitVector(double angles[D]) {
		Vector<D, T> result;
		result[0] = static_cast<T>(cos(angles[0]));
		for (size_t i = 1u; i < D; ++i) {

			result[i] = (i + 1u != D) ?
				static_cast<T>(cos(angles[i])) :
				1;

			for (size_t j = 0u; j < D - 1u; ++j) {
				result[i] *= static_cast<T>(sin(angles[j]));
			}
		}
		return result;
	}
	static Vector<2, T> createUnitVector(double angles) { // i'm not doing all the shit above for 2d
		return {
			static_cast<T>(cos(angles)),
			static_cast<T>(sin(angles))
		};
	}
	template<typename V>
	static bool equalf(const V& A, const V& B, float eps = FLT_EPSILON) {
		return (A - B).length2() <= eps * eps;
	}
	template<typename V>
	static bool equal(const V& A, const V& B, double eps = DBL_EPSILON) {
		return (A - B).length2() <= eps * eps;
	}
	static Vector<D, T> clamp(const Vector<D, T>& V, const Vector<D, T>& min, const Vector<D, T>& max) {
		Vector<D, T> result;
		for (u32 i = 0u; i < D; ++i) {
			result[i] = std::clamp(V[i], min[i], max[i]);
		}
		return result;
	}
	static Vector<D, T> rand(
		const Vector<D, T>& min, 
		const Vector<D, T>& max, 
		std::default_random_engine rng = std::default_random_engine()
	) {
		Vector<D, T> r;

		if constexpr (std::is_integral_v<T>) {
			for (size_t i = 0u; i < D; ++i) {
				r[i] = std::uniform_int_distribution<T>(min[i], max[i])(rng);
			}
			return r;
		}
		
		for (size_t i = 0u; i < D; ++i) {
			r[i] = std::uniform_real_distribution<T>(min[i], max[i])(rng);
		}

		return r;
	}

	constexpr Vector() {
		for (size_t i = 0u; i < D; ++i) {
			this->components[i] = static_cast<T>(0);
		}
	}

	template<size_t Dp = D>
	constexpr Vector(T x, std::enable_if_t<Dp == 2, T> y) :
		__vec_member<2, T>(x, y)
	{}

	template<size_t Dp = D>
	constexpr Vector(T x, T y, T z, std::enable_if_t<Dp == 4, T> w) :
		__vec_member<4, T>(x, y, z, w)
	{}

	size_t getDimension() const {
		return D;
	}

	T& operator[](size_t i) {
		return this->components[i];
	}
	const T& operator[](size_t i) const {
		return this->components[i];
	}

	Vector<D, T>& clamp(const Vector<D, T>& min, const Vector<D, T>& max) {
		for (u32 i = 0u; i < D; ++i) {
			this->components[i] = std::clamp(this->components[i], min[i], max[i]);
		}
		return *this;
	}

	template<typename U>
	bool inRect(const Vector<D, U>& pos, const Vector<D, U>& size) const {
		for (size_t i = 0u; i < D; ++i) {
			if (!(
					static_cast<T>(pos[i]) < this->components[i] && 
					this->components[i] < static_cast<T>(pos[i] + size[i])
				)) 
			{
				return false;
			}
		}

		return true;
	}

	T length() const {
		T result = 0;
		for (size_t i = 0u; i < D; ++i) {
			result += this->components[i] * this->components[i];
		}
		return sqrt(result);
	}

	T length2() const {
		T result = 0;
		for (size_t i = 0u; i < D; ++i) {
			result += this->components[i] * this->components[i];
		}
		return result;
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, double> angleX() const noexcept {
		return std::atan2(this->y, this->x);
	}

	template<typename U, size_t Dp = D>
	std::enable_if_t<Dp == 2, double> angleTo(const Vector<2U, U>& other) const noexcept {
		return std::atan2(other.y - this->y, other.x - this->x);
	}

	template<typename U, size_t Dp = D>
	std::enable_if_t<Dp == 2, double> angleFrom(const Vector<2U, U>& other) const noexcept {
		return std::atan2(this->y - other.y, this->x - other.x);
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, double> pseudoAngleX() const noexcept {
		auto dx = this->x;
		auto dy = this->y;
		return std::copysign(1.0 - dx / (std::fabs(dx) + fabs(dy)), dy);
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, double>
	pseudoAngleTo(const Vector<2U, T>& other) const noexcept {
		return (other - *this).pseudoAngleX();
	}

	template<size_t Dp = D>
	std::enable_if_t<Dp == 2, double>
		pseudoAngleFrom(const Vector<2U, T>& other) const noexcept {
		return (*this - other).pseudoAngleX();
	}

	Vector<D, T>& normalize() {
		const auto& l = length();
		if (l == 0) return *this;
		for (size_t i = 0u; i < D; ++i) {
			this->components[i] /= l;
		}
		return *this;
	}

	Vector<D, T> round(T magnitude) {
		Vector<D, T> results;
		for (size_t i = 0; i < D; ++i) {
			results[i] = static_cast<T>(
				std::round(this->components[i] / magnitude) * magnitude
			);
		}
		return results;
	}

	template<typename U>
	Vector<D, T> operator*(const U& scalaire) const {
		static_assert(std::is_scalar<U>::value, "need to be a scalar");
		Vector<D, T> result;
		for (size_t i = 0; i < getDimension(); ++i) {
			result[i] = static_cast<T>(this->components[i] * scalaire);
		}

		return result;
	}
	template<typename U>
	Vector<D, T> operator/(const U& scalaire) const {
		static_assert(std::is_scalar<U>::value);
		Vector<D, T> result;
		for (size_t i = 0; i < getDimension(); ++i) {
			result[i] = static_cast<T>(this->components[i] / scalaire);
		}

		return result;
	}

	template<typename U>
	Vector<D, T> operator+(const U& other) const {
		Vector<D, T> result;
		for (size_t i = 0; i < getDimension(); ++i) {
			result[i] = static_cast<T>(this->components[i] + other[i]);
		}

		return result;
	}
	template<typename U>
	Vector<D, T> operator-(const U& other) const {
		Vector<D, T> result;
		for (size_t i = 0; i < getDimension(); ++i) {
			result[i] = static_cast<T>(this->components[i] - other[i]);
		}

		return result;
	}

	template<typename U>
	Vector<D, T>& operator+=(const U& other) {
		for (size_t i = 0; i < getDimension(); ++i) {
			this->components[i] += static_cast<T>(other[i]);
		}
		return *this;
	}
	template<typename U>
	Vector<D, T>& operator*=(const U& scalaire) {
		static_assert(std::is_scalar<U>::value);
		for (size_t i = 0; i < getDimension(); ++i) {
			this->components[i] *= static_cast<T>(scalaire);
		}
		return *this;
	}

	template<typename U>
	bool operator==(const Vector<D, U>& other) const {
		for (size_t i = 0u; i < D; ++i) {
			if (this->components[i] != other.components[i])
				return false;
		}
		return true;
	}
	template<typename U>
	bool operator!=(const Vector<D, U>& other) const {
		return !(this->operator==(other));
	}

	template<typename U>
	explicit operator const Vector<D, U>() const {
		Vector<D, U> results;
		for (size_t i = 0u; i < D; ++i) {
			results[i] = static_cast<U>(this->components[i]);
		}
		return results;
	}

	explicit operator const std::string() const {
		std::string r = std::to_string(this->components[0]);
		for (size_t i = 1u; i < D; ++i) {
			r += ' ';
			r += std::to_string(this->components[i]);
		}
		return r;
	}

	//SFML compatibility stuff
#ifdef SFML_VECTOR2_HPP
	template<typename U, size_t Dp = D>
	constexpr Vector(const sf::Vector2<U>& p, std::enable_if_t<Dp == 2, T>* = nullptr) :
		__vec_member<2, T>((T)p.x, (T)p.y)
	{}

	template<typename U>
	operator const sf::Vector2<U>() const {
		static_assert(D == 2);
		return {
			static_cast<U>(this->x),
			static_cast<U>(this->y)
		};
	}

	template<typename U = T>
	operator const std::enable_if_t<
		std::is_same_v<U, T> && std::is_floating_point_v<T> && D == 4, 
		sf::Color
	>() const {
		return sf::Color(COLOR_UNROLL(
			((Vector<4U, u08>)(this->operator*(255)))
		));
	}

	template<typename U>
	Vector<D, T>& operator=(const sf::Vector2<U>& other) {
		static_assert(D == 2);
		this->x = static_cast<T>(other.x);
		this->y = static_cast<T>(other.y);
		return *this;
	}

	template<typename U>
	bool operator==(const sf::Vector2<U>& other) const {
		static_assert(D == 2);
		return this->components[0] == other.x && this->components[1] == other.y;
	}
	template<typename U>
	bool operator!=(const sf::Vector2<U>& other) const {
		static_assert(D == 2);
		return !this->operator==(other);
	}

	static void renderLine(
		sf::RenderTarget& target,
		Vector<2U, T> A,
		Vector<2U, T> B,
		Vector4d colorA,
		Vector4d colorB
	) {
		sf::Vertex vertex[2] {
			sf::Vertex(A, (sf::Color)colorA),
			sf::Vertex(B, (sf::Color)colorB)
		};

		target.draw(vertex, 2, sf::Lines);
	}
	static void renderLine(
		sf::RenderTarget& target,
		Vector<2U, T> A,
		Vector<2U, T> B,
		Vector4d colorA
	) {
		Vector<2U, T>::renderLine(target, A, B, colorA, colorA);
	}

	void render(
		sf::RenderTarget& target, Vector4d colorA, Vector4d colorB = colorA
	) const {
		Vector<2U, T>::renderLine(target, { (T)0, (T)0 }, *this, colorA, colorB);
	}	
	void render(sf::RenderTarget& target, Vector4d colorA) const {
		render(target, colorA, colorA);
	}

	void plot(
		sf::RenderTarget& target,
		float r,
		Vector4d fill,
		Vector4d outline,
		float thick
	) const {
		sf::CircleShape circle(r);
		circle.setOrigin(r, r);
		circle.setPosition((float)this->x, (float)this->y);
		circle.setFillColor(fill);
		circle.setOutlineColor(outline);
		circle.setOutlineThickness(thick);
		target.draw(circle);
	}

	void drawArrow(
		sf::RenderTarget& target, float thick, Vector4d color, Vector2<T> offset = { 0, 0 }
	) const noexcept {
		sf::RectangleShape stick{ sf::Vector2f{(float)length() * 0.9f, thick * 2} };
		stick.setOrigin(0, thick);
		stick.setPosition(offset);
		stick.setRotation((float)(angleX() * Common::RAD_2_DEG));

		sf::CircleShape triangle{ 3 * thick * std::tanf((float)Common::PI / 3.f), 3};
		triangle.setOrigin(triangle.getRadius(), triangle.getRadius());
		triangle.setRotation(stick.getRotation() + 90);
		triangle.setPosition(
			(Vector2f)offset + 
			(Vector2f::createUnitVector(angleX()) * 
				((float)length() * 0.9f - triangle.getRadius() * (1 - 1 / 3.f)))
		);
		
		stick.setFillColor(color);
		triangle.setFillColor(color);

		target.draw(stick);
		target.draw(triangle);
	}

#endif
};

template<size_t D, typename T, typename U>
Vector<D, T> operator*(U scalar, const Vector<D, T>& vec) {
	return vec * scalar;
}

namespace std {
	template<size_t D, typename T>
	struct hash<Vector<D, T>> {
		std::size_t operator()(const Vector<D, T>& k) const {
			return hash_combine_vector(0, k.components, D);
		}
	};
}
#pragma warning(pop)