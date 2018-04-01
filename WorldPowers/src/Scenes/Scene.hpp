#pragma once

#include <SFML/Graphics.hpp>

#include "Common.hpp"

class Game;
class Scene {
public:
	Scene(Game* g);
	virtual ~Scene() {};

	virtual void init() = 0;
	virtual void enter() = 0;
	virtual void leave() = 0;

	virtual void input() = 0;
	virtual void render(sf::RenderTarget& target) = 0;
	virtual void update(dt_t dt) = 0;

private:
	Game* _game;
};