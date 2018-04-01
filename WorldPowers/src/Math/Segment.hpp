#pragma once

#include "./Vector.hpp"

template<class P>
struct Segment {
	template<typename T>
	static bool equal(const Segment<T>& A, const Segment<T>& B, T eps) {
		return (Vector2<T>::equal(A.a, B.a, eps) && Vector2<T>::equal(A.b, B.b, eps)) ||
				(Vector2<T>::equal(A.b, B.a, eps) && Vector2<T>::equal(A.a, B.b, eps));
	}
	
	Vector2<P> a;
	Vector2<P> b;

	bool intersect(const Segment<P>& other, Vector2<P>* i = nullptr) const {
		Vector2<P> s1 = b - a;
		Vector2<P> s2 = other.b - other.a;

		double dt = -s2.x * s1.y + s1.x * s2.y;
		if (dt == 0) return false;

		double s, t;
		s = (-s1.y * (a.x - other.a.x) + s1.x * (a.y - other.a.y)) / dt;
		t = (+s2.x * (a.y - other.a.y) - s2.y * (a.x - other.a.x)) / dt;

		if (s >= .0 && s <= 1.0 && t >= .0 && t <= 1.0) {
			if (i) *i = a + t * s1;
			return true;
		}
		return false;
	}
};

using Segmentd = Segment<double>;