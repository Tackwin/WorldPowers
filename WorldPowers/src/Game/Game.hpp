#pragma once

#include <stack>
#include <memory>

#include <SFML/Graphics.hpp>

#include "./../Managers/MemoryManager.hpp"
#include "./../Scenes/Scene.hpp"
#include "./../Map/Map.hpp"
#include "Common.hpp"

class Game {
public:
	Game();
	~Game();

	void input();
	void render(sf::RenderTarget& target);
	void update(dt_t dt);

private:
	std::stack<MM::unique_ptr<Scene>> _scenes;

	Map _map;

};

