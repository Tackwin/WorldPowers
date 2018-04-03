#pragma once
#include <unordered_map>
#include <SFML/Graphics.hpp>

#include "Common.hpp"

#include "./../Managers/MemoryManager.hpp"

#include "./../Math/Rectangle.hpp"
#include "./../Math/Polygon.hpp"
#include "./../Math/Vector.hpp"
#include "./../Math/Graph.hpp"

#include "./../3rd/jc_voronoi.h"
#include "./../3rd/FastNoise.h"

#include "./../Data/Datum.hpp"

#include "./../Structures/QuadTree.hpp"

#include "./Canton.hpp"

class Map {
public:

	enum RenderFlags : u08 {
		VOID = 0,
		ELEVATION = 1,
		DOWNHILLNESS = ELEVATION << 1
	};

	Map(Vector2d size);
	~Map();

	Map(const Map&) = delete;
	Map& operator=(const Map&) = delete;

	void build(double poisson_r);

	void render(sf::RenderTarget& target, RenderFlags flags = RenderFlags::VOID) const;
	void update(dt_t);

	double getElevation(Vector2d p) const;

	sf::View& getView();

	bool isBuilded() const;

	Rectangle2d getScreenRec() const noexcept;
	Vector2d screenToMap(Vector2d p) const;

	const Canton& getCanton(ID id) const;
	const Frontier& getFrontier(ID id) const;
private:

	void clear();

	void generateVoronoi(double r);
	void generateNoise();

	void computeTextureElevation(double LOD);

	void renderDownhill(sf::RenderTarget& target) const noexcept;
	void renderElevation(sf::RenderTarget& target) const;

	struct Info {
		Vector2d size;
		jcv_diagram diagram;
		bool builded;

		std::unordered_map<ID, Canton> cantons;
		std::unordered_map<ID, Frontier> frontiers;
		std::unordered_map<ID, Tripoint> tripoints;

		u32 noiseSeed;
		std::vector<std::tuple<FastNoise::NoiseType, double, double>> weights;
	} _info;

	sf::View _view;

	mutable FastNoise _noise;

	sf::Texture _elevationTexture;
	Canton* _hovered{ nullptr };

	friend class Canton;
};