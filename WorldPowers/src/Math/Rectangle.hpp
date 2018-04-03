#pragma once

#include "Vector.hpp"
#include "Segment.hpp"
#pragma warning(push)
#pragma warning(disable: 4201)

template<typename T>
struct Rectangle {
	union {
		struct {
			Vector<2, T> pos;
			Vector<2, T> size;
		};
		struct {
			T x;
			T y;
			T w;
			T h;
		};
	};

	Rectangle() : pos(), size() {}

	Rectangle(const Vector<2, T>& pos, const Vector<2, T>& size) :
		pos(pos),
		size(size) 
	{}


	bool intersect(const Rectangle<T>& other) const {
		return !(
				pos.x + size.x < other.pos.x || pos.x > other.pos.x + other.size.x ||
				pos.y + size.y < other.pos.y || pos.y > other.pos.y + other.size.y
			);
	}

	Vector<2, T> center() const {
		return pos + size / 2;
	}

	T bot() const {
		return pos.y + size.y;
	}

	template<typename U>
	bool contains(const Vector<2, U>& p) const {
		return p.inRect(pos, size);
	}

	std::tuple<Rectangle<T>, Rectangle<T>, Rectangle<T>, Rectangle<T>>
	divide() const {
		Rectangle<T> a(pos, size / 2);
		Rectangle<T> b(pos + Vector<2U, T>(size.x / 2, 0), size / 2);
		Rectangle<T> c(pos + Vector<2U, T>(0, size.y / 2), size / 2);
		Rectangle<T> d(pos + size / 2, size / 2);
		return { a, b, c, d };
	}

	bool containsRect(const Rectangle<T>& rec) const {
		return 
			x		<= rec.x			&& y		<= rec.y		&&
			x + w	>= rec.x + rec.w	&& y + h	>= rec.y + rec.h;
	}

	bool circleIntersect(const Vector<2U, T>& c, T r) const {
		auto dx = c.x - std::max(pos.x, std::min(c.x, pos.x + size.x));
		auto dy = c.y - std::max(pos.y, std::min(c.y, pos.y + size.y));
		return (dx * dx + dy * dy) < (r * r);
	}

	bool containsCircle(const Vector<2U, T>& c, T r) const {
		return	pos.x < c.x - r && c.x + r < pos.x &&
				pos.y < c.y - r && c.y + r < pos.y;
	}

	bool containedInCircle(const Vector<2U, T>& c, T r) const {
		const auto& r2 = r * r;
		auto A = pos;
		auto B = pos;
		B.x += size.x;
		auto C = pos;
		C.y += size.y;
		auto D = pos + size;

		return	(A - c).length2() < r2 && 
				(B - c).length2() < r2 &&
				(C - c).length2() < r2 &&
				(D - c).length2() < r2;
	}

	bool lineIntersect(const Segment<T>& seg, Vector<2U, T>* i = nullptr) const {
		decltype(seg) top	{ pos, { pos.x + size.x, pos.y } };
		decltype(seg) right	{ { pos.x + size.x, pos.y }, pos + size };
		decltype(seg) bot	{ pos + size, { pos.x, pos.y + size.y } };
		decltype(seg) left	{ { pos.x, pos.y + size.y }, pos };

		return	seg.intersect(top, i) ||
				seg.intersect(right, i) ||
				seg.intersect(bot, i) ||
				seg.intersect(left, i);
	}

	// https://web.archive.org/web/20151004071751/https://i.stack.imgur.com/2pPYs.gif
	Vector2d closestPoint(const Vector<2U, T>& p) const {
		if (p.x < pos.x && p.y < pos.y) return pos;
		if (p.x < pos.x && p.y > pos.y + size.y) return { pos.x, p.y };
		if (p.x < pos.x) return { pos.x, pos.y + size.y };

		if (p.x < pos.x + size.x && p.y < pos.y) return { p.x, pos.y };
		if (p.x > pos.x + size.x && p.y < pos.y) return { pos.x + size.x, pos.y };

		if (p.x < pos.x + size.x && p.y > pos.y + size.y) return { p.x, pos.y + size.y };
		if (p.x > pos.x + size.x && p.y > pos.y + size.y) return pos + size;

		if (p.x > pos.x + size.x) return { pos.x + size.x, p.y };

		auto dTop = std::abs(pos.y - p.y);
		auto dRight = std::abs(pos.x + size.x - p.x);
		auto dLeft = std::abs(pos.x - p.x);
		auto dBot = std::abs(pos.y + size.y - p.y);

		if (dTop < dBot && dTop < dRight && dTop < dLeft) return { p.x, pos.y };
		if (dTop < dBot && dRight < dTop && dRight < dLeft) return { pos.x + size.x, p.y };
		if (dTop < dBot && dLeft < dTop && dLeft < dRight) return { pos.x, p.y };

		if (dTop > dBot && dBot < dRight && dBot < dLeft) return { p.x, pos.y + size.y };
		if (dTop > dBot && dRight < dBot && dRight < dLeft) return { pos.x + size.x, p.y };
		if (dTop > dBot && dLeft < dBot && dLeft < dRight) return { pos.x, p.y };

		// YOU SHOULD NOT GO HERE;
		*(void*)0;
		return { 0, 0 };
	}

#ifdef SFML_GRAPHICS_HPP
	void render(sf::RenderTarget& target, Vector4d in, Vector4d out, float thick) const {
		sf::RectangleShape shape(size);
		shape.setPosition(pos);
		shape.setFillColor(in);
		shape.setOutlineColor(out);
		shape.setOutlineThickness(thick);

		target.draw(shape);
	}
#endif
};

template<typename T>
using Rectangle2 = Rectangle<T>;
using Rectangle2f = Rectangle2<float>;
using Rectangle2d = Rectangle2<double>;
#pragma warning(pop)
