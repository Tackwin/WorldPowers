#pragma once
#include "Scene.hpp"

#include <SFML/Graphics.hpp>

#include "Common.hpp"

class Game;
class Map;

class MapScene : public Scene {
public:
	MapScene(Game* g, Map* m);
	virtual ~MapScene() override;

	virtual void init() override {};
	virtual void enter() override {};
	virtual void leave() override {};

	virtual void input() override;
	virtual void update(dt_t dt) override;
	virtual void render(sf::RenderTarget& target) override;

private:

	void updateImGui();

	Map* _map = nullptr;

	float _viewScale = 1.f;

	struct ImGui_data {
		float poisson_r = 1.f;

		bool render_elevation = false;
	} _imgui_data;
};

