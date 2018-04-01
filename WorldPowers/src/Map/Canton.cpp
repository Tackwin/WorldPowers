#include "Canton.hpp"
#include <algorithm>
#include <iostream>

#include "./Map.hpp"

const Vector4d Canton::__colors__[] = {
	{0.06, 0.37, 0.61, 1.0},
	{0.70, 0.56, 0.29, 1.0},
	{0.42, 0.36, 0.22, 1.0},
	{0.31, 0.33, 0.35, 1.0},
};

Canton::Canton() : _id(ID::construct()) {
}
Frontier::Frontier() : id(ID::construct()) {}
Tripoint::Tripoint() : id(ID::construct()) {}

Canton::Canton(Map* map) : _map(map), _id(ID::construct()) {

}

void Canton::addFrontier(ID id) {
	_frontiers.emplace(id);
}

void Canton::setEdges(const Polygon<double>& edges) {
	_edges = edges;
}

void Canton::setSite(Vector2d site) {
	_site = site;
}

void Canton::render(sf::RenderTarget& target) const {
	Vector4d in;
	Vector4d out{ 0.7, 0.7, 0.7, 0.1 };
	if (_map->getElevation({ _site.x, _site.y }) < 0.0) {
		in = __colors__[COLORS::WATER];
	}
	else {
		in = __colors__[COLORS::LAND];
	}

	if (_map->_hovered == this) {
		in.r = std::sqrt(in.r);
		in.g = std::sqrt(in.g);
		in.b = std::sqrt(in.b);

		out.r = std::sqrt(out.r);
		out.g = std::sqrt(out.g);
		out.b = std::sqrt(out.b);
		out.a = 1.0;
	}

	_edges.render(target, in, out, 0.1f);
}

Vector2d Canton::getSite() const {
	return _site;
}

const std::unordered_set<ID>& Canton::getFrontiers() const {
	return _frontiers;
}

const Polygon<double>& Canton::getEdges() const {
	return _edges;
}

bool Canton::is(ID i) const {
	return _id == i;
}

ID Canton::id() const {
	return _id;
}