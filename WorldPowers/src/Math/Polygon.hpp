#pragma once

#include <vector>
#include <algorithm>
#include <numeric>

#include <SFML/Graphics.hpp>

#include "./Rectangle.hpp"
#include "./Vector.hpp"

template<typename T>
struct Polygon {
	using Vec = Vector2<T>;

	std::vector<Vec> vertices;

	Polygon() = default;

	Polygon(const std::vector<Vec>& vec) : vertices(vec) {
		Vec m = massCenter();
		std::sort(std::begin(vertices), std::end(vertices), 
			[m](const Vec& A, const Vec& B) -> bool {
				return (A - m).angleX() < (B - m).angleX();
			}
		);

	}

	Vec massCenter() const {
		if (!_massCenterCacheValidation) {

			_cachedMassCenter = std::accumulate(
				std::begin(vertices), std::end(vertices), Vec { (T)0.0, (T)0.0 }
			);
			_cachedMassCenter.x /= vertices.size();
			_cachedMassCenter.y /= vertices.size();
			_massCenterCacheValidation = true;
		}

		return _cachedMassCenter;
	}

	Rectangle2<T> boundingBox() const {
		if (!_bondingBoxCacheValidation) {
			Vec min = vertices[0];
			Vec max = vertices[0];
			for (auto& v : vertices) {
				min.x = v.x < min.x ? v.x : min.x;
				min.y = v.y < min.y ? v.y : min.y;

				max.x = v.x > max.x ? v.x : max.x;
				max.y = v.y > max.y ? v.y : max.y;
			}

			_cachedBondingBox.pos = min;
			_cachedBondingBox.size = max - min;

			_bondingBoxCacheValidation = true;
		}
		return _cachedBondingBox;
	}

	bool interesctRect(Rectangle2<T>&  rec, Vector2<T>* it = nullptr) const {
		for (size_t i = 0u; i < vertices.size(); ++i) {
			Segment<T> seg{ vertices[i], vertices[(i + 1) % vertices.size()] };

			if (rec.lineIntersect(seg, it)) return true;
		}
		return false;
	}

	bool inRect(Rectangle2<T>& rec) const {
		return rec.containsRect(boundingBox());
	}

	Vec& operator[](std::size_t i) {
		_massCenterCacheValidation = false;
		return vertices[i];
	}
	const Vec& operator[](std::size_t i) const {
		return vertices[i];
	}

	size_t size() const { return vertices.size(); }

	bool operator==(const Polygon<T>& other) const {
		for (size_t i = 0; i < vertices.size(); ++i) {
			if (*this[i] != other[i]) return false;
		}
		return true;
	}

	bool contain(Vector2d p) const {
		const Rectangle2<T> box = boundingBox();
		if (!box.contains(p)) return false;

		u32 c = 1u;
		Vector2d out = box.pos - Vector2d{ 1.0, 1.0 };
		for (u32 i = 0; i < vertices.size(); ++i) {
			Segment<double> seg{ 
				(Vector2d)vertices[i],
				(Vector2d)vertices[(i + 1) % vertices.size()]
			};
			if (seg.intersect({ out, p })) c++;
		}

		return (c % 2) == 0;
	}

	void render(sf::RenderTarget& target, Vector4d color) const {
		sf::ConvexShape shape(vertices.size());
		size_t i = 0;
		for (auto& v : vertices)
			shape.setPoint(i++, v);

		shape.setFillColor(color);

		target.draw(shape);
	}
	void render(
		sf::RenderTarget& target, 
		Vector4d fill, 
		Vector4d outline, 
		float thickness = 2
	) const {
		sf::ConvexShape shape(vertices.size());
		size_t i = 0;
		for (auto& v : vertices)
			shape.setPoint(i++, v);

		shape.setOutlineThickness(thickness);
		shape.setFillColor(fill);
		shape.setOutlineColor(outline);

		target.draw(shape);
	}

private:
	mutable Rectangle2<T> _cachedBondingBox{};
	mutable bool _bondingBoxCacheValidation{ false };

	mutable Vec _cachedMassCenter{};
	mutable bool _massCenterCacheValidation{ false };
};