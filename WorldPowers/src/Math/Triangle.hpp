#pragma once
#include "Vector.hpp"

class Triangle {

public:

	Triangle(const Vector2d& A, const Vector2d& B, const Vector2d& C);

	Vector2d A;
	Vector2d B;
	Vector2d C;
};