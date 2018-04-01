#include <iostream>
#include <SFML/Graphics.hpp>

#include "Common.hpp"


#include "Managers/InputsManager.hpp"
#include "Managers/MemoryManager.hpp"

#include "Time/Clock.hpp"

#include "imgui.h"
#include "imgui-SFML.h"

#include "./Game/Game.hpp"

int main(int, char**) {

	MM::I().initialize_buffer(1'024 * 1'024);

	sf::RenderWindow window(sf::VideoMode(900, 600), "WorldPowers" __TIME__);
	ImGui::CreateContext();
	ImGui::SFML::Init(window);

	Game game;

	Clock dtClock;
	while (window.isOpen()) {
		auto dt = dtClock.restart();

		InputsManager::update(window);
		game.input();

		ImGui::SFML::Update(window, sf::seconds((float)dt));
		game.update((dt_t)dt);

		window.clear();
		game.render(window);
		ImGui::SFML::Render(window);
		window.display();
	}
	
	return 0;
}

// Just redefining some legacy stuff
extern "C" FILE* __cdecl __iob_func(void) {
	static FILE _iob[] = { *stdin, *stdout, *stderr };
	return _iob;
}