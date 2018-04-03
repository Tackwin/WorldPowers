#include "Map.hpp"

#include <algorithm>
#include <set>

#include "./../Game/ID.hpp"
#include "./../Math/Algorithm.hpp"
#include "./../Managers/InputsManager.hpp"

Map::Map(Vector2d size) {
	_info.size = size;
	_elevationTexture.create((u32)_info.size.x, (u32)_info.size.y);
}

Map::~Map() {
	if (_info.builded)
		jcv_diagram_free(&_info.diagram);
}


void Map::build(double poisson_r) {
	if (isBuilded()) {
		clear();
	}
	_info.builded = true;

	generateVoronoi(poisson_r);
	generateNoise();

	computeTextureElevation(1);
	computeElevation();
}

void Map::render(sf::RenderTarget& target, Map::RenderFlags flag) const {
	auto oldView = target.getView();

	target.setView(_view);

	auto scope = getScreenRec();

	sf::CircleShape pointShape(0.2f);
	pointShape.setOrigin(0.2f, 0.2f);
	pointShape.setFillColor(sf::Color(255, 255, 255, 100));

	for (auto& [_, c] : _info.cantons) {
		_;

		if (!(c.getEdges().inRect(scope) || c.getEdges().interesctRect(scope))) continue;
		if (_hovered == &c) continue;

		c.render(target);
		pointShape.setPosition(c.getSite());
		target.draw(pointShape);
	}

	if (_hovered) {
		_hovered->render(target);
		pointShape.setPosition(_hovered->getSite());
		target.draw(pointShape);

		auto A = _hovered->getSite();
		for (auto& f : _hovered->getFrontiers()) {
			auto cantons = _info.frontiers.at(f).cantons;
			ID n = xstd::other_one_pair(cantons, _hovered->id());

			auto B = std::find_if(std::begin(_info.cantons), std::end(_info.cantons),
				[n](const auto& A) -> bool { return A.second.is(n); }
			);
			if (B == std::end(_info.cantons)) continue;

			Vector2d::renderLine(target, A, (_info.cantons.at(B->first)).getSite(), { 1.0, 0.3, 0.3, 100 });
		}
	}

	if (flag & RenderFlags::ELEVATION) {
		renderElevation(target);
	}
	if (flag & RenderFlags::DOWNHILLNESS) {
		renderDownhill(target);
	}

	scope.render(target, { 0, 0, 1, 0.1 }, { 1, 1, 0, 1 }, 5);

	target.setView(oldView);
}

void Map::renderDownhill(sf::RenderTarget& target) const noexcept {
	auto scope = getScreenRec();

	for (auto& [_, c] : _info.cantons) {
		_;

		if (!(c.getEdges().inRect(scope) || c.getEdges().interesctRect(scope))) continue;
		if (_hovered == &c) continue;

		c.renderDownhill(target);
	}
}

void Map::renderElevation(sf::RenderTarget& target) const {
	sf::Sprite spr(_elevationTexture);
	spr.setScale(
		(float)_info.size.x / _elevationTexture.getSize().x, 
		(float)_info.size.y / _elevationTexture.getSize().y
	);
	target.draw(spr);
}

void Map::update(dt_t) {
	_hovered = nullptr;
	if (!_info.builded) return;

	for (auto& [_, c] : _info.cantons) {
		_;
		if (c.getEdges().contain(screenToMap((Vector2d)IM::getMouseScreenPos()))) {
			_hovered = &c;
			break;
		}
	}
}

void Map::generateVoronoi(double r) {
	auto vertices = math::poissonDiscSampling(_info.size, r);

	std::vector<jcv_point> ps(vertices.size());
	for (size_t i = 0; i < vertices.size(); ++i) {
		ps[i].x = vertices[i].x;
		ps[i].y = vertices[i].y;
	}

	jcv_rect rect;
	rect.min = jcv_point { 0.0, 0.0 };
	rect.max = jcv_point { _info.size.x, _info.size.y };
	
	jcv_diagram_generate(
		ps.size(),
		ps.data(),
		&rect,
		&_info.diagram
	);

	auto sites = jcv_diagram_get_sites(&_info.diagram);

	std::unordered_map<Vector2d, ID> tripointMapIndex;
	std::unordered_map<std::pair<ID, ID>, ID> tripontsToFrontierMap;

	auto edge = jcv_diagram_get_edges(&_info.diagram);
	while (edge) {
		Tripoint A;
		Tripoint B;

		A.pos = { edge->pos[0].x, edge->pos[0].y };
		B.pos = { edge->pos[1].x, edge->pos[1].y };

		tripointMapIndex[A.pos] = A.id;
		tripointMapIndex[B.pos] = B.id;

		_info.tripoints[A.id] = A;
		_info.tripoints[B.id] = B;

		edge = edge->next;
	}

	edge = jcv_diagram_get_edges(&_info.diagram);
	while (edge) {
		Frontier F;
		auto A = tripointMapIndex.at(Vector2d{ edge->pos[0].x, edge->pos[0].y });
		auto B = tripointMapIndex.at(Vector2d{ edge->pos[1].x, edge->pos[1].y });

		F.tripoints.first = A;
		F.tripoints.second = B;

		tripontsToFrontierMap[std::minmax({A, B})] = F.id;

		_info.frontiers[F.id] = F;

		_info.tripoints.at(A).frontiers.emplace(F.id);
		_info.tripoints.at(B).frontiers.emplace(F.id);

		edge = edge->next;
	}

	for (i32 i = 0; i < _info.diagram.numsites; ++i) {
		Canton c(this);

		auto s = &sites[i];
		std::unordered_set<Vector2d> ver;
		c.setSite({ s->p.x, s->p.y });

		auto const* e = s->edges;
		while (e) {
			auto A = Vector2d{ e->edge->pos[0].x, e->edge->pos[0].y };
			auto B = Vector2d{ e->edge->pos[1].x, e->edge->pos[1].y };

			auto Aid = tripointMapIndex.at(A);
			auto Bid = tripointMapIndex.at(B);
			auto Fid = tripontsToFrontierMap.at(std::minmax({ Aid, Bid }));

			_info.tripoints.at(Aid).cantons.emplace(c.id());
			_info.tripoints.at(Bid).cantons.emplace(c.id());
			
			auto& F = _info.frontiers.at(Fid).cantons;
			if (F.first == c.id() || F.second == c.id()) {}
			else if ((bool)F.first) {
				F.second = c.id();
			}
			else {
				F.first = c.id();
			}

			ver.insert(A);
			ver.insert(B);
			c.addFrontier(Fid);
			e = e->next;
		}

		Polygon<double> p(std::vector<Vector2d>(std::begin(ver), std::end(ver)));
		c.setEdges(p);

		_info.cantons[c.id()] = c;
	}
	printf("");
}

void Map::generateNoise() {
	_info.noiseSeed = (u32)time((time_t*)0);
	_noise.SetSeed(_info.noiseSeed);
	_info.weights = {
		{FastNoise::Simplex, 0.005, 0.2},
		{FastNoise::Simplex, 0.003, 0.35},
		{FastNoise::Simplex, 0.001, 0.25},
		{FastNoise::Perlin, 0.01, 0.1},
		{FastNoise::Perlin, 0.05, 0.1}
	};
}

void Map::clear() {
	_info.builded = false;
	_info.cantons.clear();
	_info.frontiers.clear();
	_info.tripoints.clear();
	jcv_diagram_free(&_info.diagram);
	memset(&_info.diagram, 0, sizeof _info.diagram);
	_hovered = nullptr;
}

sf::View& Map::getView() {
	return _view;
}

bool Map::isBuilded() const {
	return _info.builded;
}

void Map::computeTextureElevation(double LOD) {
	sf::Image img;
	img.create(
		(u32)std::ceil(_info.size.x / LOD), 
		(u32)std::ceil(_info.size.y / LOD)
	);
	for (u32 x = 0; x < img.getSize().x; ++x) {
		for (u32 y = 0; y < img.getSize().y; ++y) {
			Vector2d p;
			p.x = LOD * x;
			p.y = LOD * y;

			Vector4d color{ 1.0, 1.0, 1.0, 1.0 };
			color *= getElevation(p) / 2 + 0.5;
			color.a = std::pow(color.a, 1/3.0);
			auto c = (Vector4<u08>)(color * 255);

			img.setPixel(x, y, sf::Color(COLOR_UNROLL(c)));
		}
	}

	_elevationTexture.create(img.getSize().x, img.getSize().y);
	_elevationTexture.update(img);
}
double Map::getElevation(Vector2d p) const {
	if (!_info.builded) return 0.0;

	double sum = 0.0;
	for (auto&[T, f, w] : _info.weights) {
		_noise.SetNoiseType(T);
		_noise.SetFrequency(f);
		switch (T)
		{
		case FastNoise::Simplex:
			sum += w * atan(3 * _noise.GetSimplex(p.x, p.y));
			break;
		case FastNoise::Perlin:
			sum += w * atan(3 * _noise.GetPerlin(p.x, p.y));
			break;
		default:
			break;
		}
	}

	return 2 * sum / Common::PI;
}

Vector2d Map::screenToMap(Vector2d p) const {
	auto viewport = _view.getViewport();
	viewport.left *= _view.getSize().x;
	viewport.top *= _view.getSize().y;
	viewport.width *= _view.getSize().x;
	viewport.height *= _view.getSize().y;

	Vector2d normalized;
	Vector2d scaledP = p;

	scaledP.x *= _view.getSize().x / Common::WINDOW_WIDTH;
	scaledP.y *= _view.getSize().y / Common::WINDOW_HEIGHT;

	normalized.x = -1.0 + 2.0 * (scaledP.x - viewport.left) / viewport.width;
	normalized.y = +1.0 - 2.0 * (scaledP.y - viewport.top) / viewport.height;

	return Vector2d(_view.getInverseTransform().transformPoint(normalized));
}

void Map::computeElevation() noexcept {
	for (auto&[_, c] : _info.cantons) {_;
		auto e = getElevation(c.getSite());
		c.setElevation(e);
	}
}

const Canton& Map::getCanton(ID id) const {
	return _info.cantons.at(id);
}
const Frontier& Map::getFrontier(ID id) const {
	return _info.frontiers.at(id);
}

Rectangle2d Map::getScreenRec() const noexcept {
	Rectangle2<double> scope;
	scope.x = (double)_view.getCenter().x - _view.getSize().x / 2.0;
	scope.y = (double)_view.getCenter().y - _view.getSize().y / 2.0;
	scope.w = (double)_view.getSize().x;
	scope.h = (double)_view.getSize().y;

	return scope;
}
