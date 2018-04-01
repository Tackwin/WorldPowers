#pragma once
#include <vector>

#include "Vector.hpp"
#include "Rectangle.hpp"

#include "Graph.hpp"

#include "./../Common.hpp"

namespace math {
	std::vector<Vector2d> poissonDiscSampling(Vector2d size, double r);
};