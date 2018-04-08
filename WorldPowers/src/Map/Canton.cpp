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
	if (isWater()) {
		in = __colors__[(u32)COLORS::WATER];
	}
	else {
		in = __colors__[(u32)COLORS::LAND];
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
void Canton::renderDownhill(sf::RenderTarget& target) const noexcept {
	if ((bool)getDownhill()) {
		auto dt = (_map->getCanton(getDownhill()).getSite() - getSite());
		dt.drawArrow(
			target, (float)_edges.boundingBox().h * 0.02f, { 0.7, 0.1, 0.1, 0.7 }, getSite()
		);
	}
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

double Canton::getElevation() const noexcept {
	return _elevation;
}
void Canton::setElevation(double elevation) noexcept {
	_elevation = elevation;
	_isWater = _elevation < 0.5;
}
void Canton::computeDownhill() const noexcept {
	if (_frontiers.empty()) return;
	auto minmax = std::minmax_element(std::begin(_frontiers), std::end(_frontiers), 
		[&](ID A, ID B) -> bool {
			auto cA = xstd::other_one_pair(_map->getFrontier(A).cantons, id());
			auto cB = xstd::other_one_pair(_map->getFrontier(B).cantons, id());
			return _map->getCanton(cA).getElevation() < _map->getCanton(cB).getElevation();
		}
	);

	auto min = xstd::other_one_pair(_map->getFrontier(*minmax.first).cantons, id());
	if (_map->getCanton(min).getElevation() < getElevation()) _downhill = min;
}

ID Canton::getDownhill() const noexcept {
	if (!_downhill) computeDownhill();
	return _downhill;
}
bool Canton::isWater() const noexcept {
	return _isWater;
}
