#include "Game.hpp"
#include "imgui.h"


#include "./../Managers/InputsManager.hpp"

#include "./../Scenes/MapScene.hpp"


Game::Game() : 
	_map({ 900, 600 })
{
	_map.getView() = sf::View({ 450, 300 }, { 900, 600 });

	_scenes.push(MM::cast_unique<Scene>(MM::make_unique<MapScene>(this, &_map).release()));
}

Game::~Game() {
}

void Game::input() {
	
	if (!_scenes.empty())
		_scenes.top()->input();
}

void Game::render(sf::RenderTarget& target) {
	auto oldView = target.getView();

	if (!_scenes.empty())
		_scenes.top()->render(target);

	target.setView(oldView);
}

void Game::update(dt_t dt) {

	if (!_scenes.empty())
		_scenes.top()->update(dt);

}