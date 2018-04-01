#include "./../Math/Algorithm.hpp"

#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <string>

#include "./../Structures/QuadTree.hpp"

using namespace math;


/*
based on... https://goo.gl/WPdEPb
*/
std::vector<Vector2d> math::poissonDiscSampling(Vector2d size, double r) {
	std::default_random_engine rng(SEED);
	constexpr double sqrt1_2 = 0.7071067811865476;

	u32 k = 10;
	double r2 = r * r;
	double R = 3 * r2;
	std::vector<Vector2d> queue;
	std::vector<Vector2d> poisson;
	QuadTree<850u, double> quad(Rectangle<double>({ 0, 0 }, size));

	const auto sample = [&queue, &poisson, &quad](Vector2d p) -> void {
		queue.push_back(p);
		poisson.push_back(p);
		quad.add(p);
	};

	sample(Vector2d::rand({ 0, 0 }, size, rng));

	auto aRange = std::uniform_real_distribution<>(0, 2 * PI);
	auto tRange = std::uniform_real_distribution<>(0, R);

	decltype(quad)::vector query;
	std::vector<const decltype(quad)*> open;

	query.reserve((u32)std::floor((size.x / r) * (size.y / r) * 2));
	open.reserve(quad.nNodes());

	while (true) {
		while (!queue.empty()) {
			u32 i = std::uniform_int_distribution<u32>(0, queue.size() - 1)(rng);
			auto s = queue[i];

			for (u32 j = 0u; j < k; ++j) {
				double a = aRange(rng);
				double t = std::sqrt(tRange(rng) + r2);
				auto p = s + t * Vector2d::createUnitVector(a);

				query.resize(0);
				open.resize(0);
				quad.noAllocQueryCircle(p, r, query, open);

				if (0 <= p.x && p.x <= size.x &&
					0 <= p.y && p.y <= size.y
				) {
					bool flag = true;
					for (auto q : query) {
						if (Vector2d::equal(p, q, r)) {
							flag = false;
							break;
						}
					}
					if (flag) sample(p);
				}
			}

			queue[i] = queue.back();
			queue.pop_back();
		}
		break;
	}

	return poisson;
}
