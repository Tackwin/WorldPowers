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
private:
	static const Vector4d __colors__[];
public:
	enum class COLORS : u32 {
		WATER,
		LAND,
		MEDIUM_MONTAINS,
		TOP_MONTAINS,
		SIZE
	};
	enum class GROUP : u32 {
		WATER,
		LAND
	};

public:

	Canton();
	Canton(Map* map);

	void render(sf::RenderTarget& target) const;
	void renderDownhill(sf::RenderTarget& target) const noexcept;

	void addFrontier(ID id);

	void setEdges(const Polygon<double>& edges);
	void setSite(Vector2d site);

	Vector2d getSite() const;
	const Polygon<double>& getEdges() const;
	const std::unordered_set<ID>& getFrontiers() const;

	bool is(ID i) const;
	ID id() const;

	double getElevation() const noexcept;
	void setElevation(double elevation) noexcept;

	ID getDownhill() const noexcept;

	bool isWater() const noexcept;

private:

	void computeDownhill() const noexcept;

	bool _isWater{ false };

	Map* _map{ nullptr };

	std::unordered_set<ID> _frontiers;
	Polygon<double> _edges;
	Vector2d _site;

	double _elevation{ 0.0 };

	mutable ID _downhill;
	ID _id;
};

struct Frontier {
	Frontier();

	std::pair<ID, ID> cantons;
	std::pair<ID, ID> tripoints;

	ID id;

	bool river{ false };
};

struct Tripoint {
	Tripoint();

	Vector2d pos;

	std::unordered_set<ID> frontiers;
	std::unordered_set<ID> cantons;

	bool river{ false };

	ID id;
};

