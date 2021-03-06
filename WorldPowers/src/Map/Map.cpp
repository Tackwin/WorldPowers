#include "Map.hpp"

#include <algorithm>
#include <set>

#include "./../Game/ID.hpp"
#include "./../Math/Algorithm.hpp"
#include "./../Managers/InputsManager.hpp"
#include "./../Time/Clock.hpp"

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
	Clock perf;
	_info.builded = true;

	generateNoise();
	generateVoronoi(poisson_r);
	generateRivers(10);

	computeTextureElevation(1);
	std::cout << perf.elapsed() << std::endl;
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
	Clock cl;

	auto vertices = math::poissonDiscSampling(_info.size, r);

	std::cout << "disk sampling: " << cl.reset() << std::endl;

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

	std::cout << "voronoi: " << cl.reset() << std::endl;

	auto sites = jcv_diagram_get_sites(&_info.diagram);

	_tripointsVectors.reserve(2 * vertices.size());
	std::unordered_map<Vector2d, ID> tripointMapIndex;
	std::unordered_map<std::pair<ID, ID>, ID> tripontsToFrontierMap;

	auto edge = jcv_diagram_get_edges(&_info.diagram);
	while (edge) {
		Tripoint A;
		Tripoint B;
		Frontier F;

		A.pos = Vector2d{ edge->pos[0].x, edge->pos[0].y }.round(0.00001);
		B.pos = Vector2d{ edge->pos[1].x, edge->pos[1].y }.round(0.00001);

		F.tripoints.first = A.id;
		F.tripoints.second = B.id;

		A.frontiers.emplace(F.id);
		B.frontiers.emplace(F.id);

		tripontsToFrontierMap.emplace(std::minmax({ A.id, B.id }), F.id);

		_info.frontiers.emplace(F.id, F);

		tripointMapIndex.emplace(A.pos, A.id);
		tripointMapIndex.emplace(B.pos, B.id);

		_info.tripoints.emplace(A.id, A);
		_info.tripoints.emplace(B.id, B);

		_tripointsVectors.emplace_back(A.id);
		_tripointsVectors.emplace_back(B.id);

		edge = edge->next;
	}
	std::cout << "tripoints: " << cl.reset() << std::endl;

	edge = jcv_diagram_get_edges(&_info.diagram);
	while (edge) {
		auto A = tripointMapIndex.at(
			Vector2d{ edge->pos[0].x, edge->pos[0].y }.round(0.00001)
		);
		auto B = tripointMapIndex.at(
			Vector2d{ edge->pos[1].x, edge->pos[1].y }.round(0.00001)
		);

		if (tripontsToFrontierMap.count(std::minmax({ A, B })) == 0) {
			Frontier F;

			F.tripoints.first = A;
			F.tripoints.second = B;

			tripontsToFrontierMap.emplace(std::minmax({ A, B }), F.id);

			_info.frontiers.emplace(F.id, F);

			_info.tripoints.at(A).frontiers.emplace(F.id);
			_info.tripoints.at(B).frontiers.emplace(F.id);
		}

		edge = edge->next;
	}
	std::cout << "frontiers: " << cl.reset() << std::endl;

	for (i32 i = 0; i < _info.diagram.numsites; ++i) {
		Canton c(this);

		auto s = &sites[i];
		c.setSite(Vector2d{ s->p.x, s->p.y }.round(0.00001));

		auto elevation = getElevation(c.getSite());
		c.setElevation(elevation);

		auto comp = [s = c.getSite()](const Vector2d& A, const Vector2d& B) -> bool {
			return s.pseudoAngleTo(A) < s.pseudoAngleTo(B);
		};
		std::set<Vector2d, decltype(comp)> cantonVertices(comp);

		auto const* e = s->edges;
		while (e) {
			auto A = Vector2d{ e->edge->pos[0].x, e->edge->pos[0].y }.round(0.00001);
			auto B = Vector2d{ e->edge->pos[1].x, e->edge->pos[1].y }.round(0.00001);

			auto Aid = tripointMapIndex.at(A);
			auto Bid = tripointMapIndex.at(B);
			auto Fid = tripontsToFrontierMap.at((std::minmax({ Aid, Bid })));

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

			cantonVertices.emplace(A);
			cantonVertices.emplace(B);
			c.addFrontier(Fid);
			e = e->next;
		}


		Polygon<double> p;
		p.vertices = std::vector<Vector2d>(
			std::begin(cantonVertices), std::end(cantonVertices)
		);
		c.setEdges(p);

		_info.cantons[c.id()] = c;
	}
	std::cout << "cantons: " << cl.reset() << std::endl;

	std::cout << std::endl
		<< "#Cantons: " << _info.cantons.size() << std::endl
		<< "#Frontiers: " << _info.frontiers.size() << std::endl
		<< "#Tripoints: " << _info.tripoints.size() << std::endl;
}

void Map::generateNoise() {
	_info.noiseSeed = (u32)time((time_t*)0);
	_noise.SetSeed(_info.noiseSeed);
	_info.weights = {
		{FastNoise::CubicFractal, 0.006, 9, 1.2, 0.9, 0.4},
		{FastNoise::CubicFractal, 0.006, 9, 1.2, 0.9, 0.4},
		{FastNoise::CubicFractal, 0.025, 6, 1.2, 0.9, 0.1},
		{FastNoise::CubicFractal, 0.040, 9, 1.2, 0.9, 0.1},
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
	for (auto&[T, f, o, l, g, w] : _info.weights) {
		_noise.SetNoiseType(T);
		_noise.SetFrequency(f);
		_noise.SetFractalOctaves(o);
		_noise.SetFractalLacunarity(l);
		_noise.SetFractalGain(g);
		switch (T)
		{
		case FastNoise::Simplex:
			sum += w * _noise.GetSimplex(p.x, p.y);
			break;
		case FastNoise::Perlin:
			sum += w * _noise.GetPerlin(p.x, p.y);
			break;
		case FastNoise::CubicFractal:
			sum += w * _noise.GetCubicFractal(p.x, p.y);
			break;
		default:
			break;
		}
	}

	auto sig = [](auto x) -> auto {
		return 1 / (1 + std::exp(-15 * x));
	};

	return sig(sum);
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

void Map::generateRivers(u32 n) {
	std::default_random_engine rng{ (u32)Common::SEED };
	std::uniform_int_distribution<u32> dist{ 0, _tripointsVectors.size() - 1};
	for (u32 i = 0; i < n; ++i) {

		ID id;
		double el{ 0.0 };
		do {
			u32 j = dist(rng);
			id = _tripointsVectors[j];
			el = getElevation(_info.tripoints[id].pos);
		} while (el < 0.75);

		generateRiverFrom(id);
	}
}

void Map::generateRiverFrom(ID tripoint) {

	auto isNextToWater = [&](ID tripoint) -> bool {
		for (auto& id : _info.tripoints[tripoint].cantons) {
			if (_info.cantons[id].isWater()) return true;
		}
		return false;
	};

	auto getDownhillFrontier = [&](ID tripointId) -> ID {
		auto lowest = ID::NILL;

		for (auto& id : _info.tripoints[tripointId].frontiers) {
			auto otherId = xstd::other_one_pair(_info.frontiers[id].tripoints, tripointId);

			if (lowest ||
					getElevation(_info.tripoints[otherId].pos) <
					getElevation(_info.tripoints[lowest].pos)
				) {
				lowest = otherId;
			}
		}

		return lowest;
	};

	ID down = tripoint;
	while (!isNextToWater(down)) {
		_info.tripoints[down].river = true;

		auto frontier = getDownhillFrontier(down);
		_info.frontiers[frontier].river = true;

		std::cout << getElevation(_info.tripoints[down].pos) << std::endl;
		down = xstd::other_one_pair(_info.frontiers[frontier].tripoints, down);
	}
}
