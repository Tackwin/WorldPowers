#pragma once
#include <unordered_set>

#include <SFML/Graphics.hpp>

#include "./../Game/ID.hpp"

#include "./../Math/Polygon.hpp"
#include "./../Math/Vector.hpp"
#include "./../Math/Segment.hpp"

class Map;
struct Frontier;
struct Tripoint;

class Canton {
public:

	Canton();
	Canton(Map* map);

	void render(sf::RenderTarget& target) const;

	void addFrontier(ID id);

	void setEdges(const Polygon<double>& edges);
	void setSite(Vector2d site);

	Vector2d getSite() const;
	const Polygon<double>& getEdges() const;
	const std::unordered_set<ID>& getFrontiers() const;

	bool is(ID i) const;
	ID id() const;

private:

	enum COLORS : u32 {
		WATER,
		LAND,
		MEDIUM_MONTAINS,
		TOP_MONTAINS,
		SIZE
	};

	static const Vector4d __colors__[];

	Map* _map{nullptr};

	std::unordered_set<ID> _frontiers;
	Polygon<double> _edges;
	Vector2d _site;

	ID _id;
};

struct Frontier {
	Frontier();

	std::unordered_set<ID> cantons;
	std::unordered_set<ID> tripoints;

	ID id;
};

struct Tripoint {
	Tripoint();

	Vector2d pos;

	std::unordered_set<ID> frontiers;
	std::unordered_set<ID> cantons;

	ID id;
};

