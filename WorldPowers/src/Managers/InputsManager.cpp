#include "InputsManager.hpp"

#include "EventManager.hpp"

#include "Common.hpp"

#include "imgui.h"
#include "imgui-SFML.h"

bool InputsManager::keyPressed[sf::Keyboard::KeyCount];
bool InputsManager::keyJustPressed[sf::Keyboard::KeyCount];
bool InputsManager::keyJustReleased[sf::Keyboard::KeyCount];

bool InputsManager::mousePressed[sf::Mouse::ButtonCount];
bool InputsManager::mouseJustPressed[sf::Mouse::ButtonCount];
bool InputsManager::mouseJustReleased[sf::Mouse::ButtonCount];

float InputsManager::wheelDelta = 0;

Vector2f InputsManager::mouseScreenPos;
Vector2f InputsManager::oldMouseScreePos;

InputsManager::InputsManager() {
}
InputsManager::~InputsManager() {
}

bool InputsManager::isKeyPressed(const sf::Keyboard::Key &key) {
	return keyPressed[key];
}
bool InputsManager::isKeyJustPressed(const sf::Keyboard::Key &key) {
	return keyJustPressed[key];
}
bool InputsManager::isKeyJustReleased(const sf::Keyboard::Key &key) {
	return keyJustReleased[key];
}

bool InputsManager::isMousePressed(const sf::Mouse::Button &button) {
	return mousePressed[button];
}
bool InputsManager::isMouseJustPressed(const sf::Mouse::Button &button) {
	return mouseJustPressed[button];
}
bool InputsManager::isMouseJustReleased(const sf::Mouse::Button &button) {
	return mouseJustReleased[button];
}

const Vector2f& InputsManager::getMouseScreenPos() {
	return mouseScreenPos;
}

Vector2f InputsManager::getDtMouseScreenPos() {
	return mouseScreenPos - oldMouseScreePos;
}

void InputsManager::update(sf::RenderWindow &window) {
	for(auto &b : keyJustPressed) {
		b = false;
	}
	for(auto &b : keyJustReleased) {
		b = false;
	}
	for(auto &b : mouseJustPressed) {
		b = false;
	}
	for(auto &b : mouseJustReleased) {
		b = false;
	}
	wheelDelta = 0;

	sf::Event event;
	while(window.pollEvent(event)) {
		ImGui::SFML::ProcessEvent(event);

		if (event.type == sf::Event::Closed)
			window.close();
		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Unknown)
				continue;

			keyJustPressed[event.key.code] = true;
			keyPressed[event.key.code] = true;

			EventManager::fire("keyPressed", { event.key.code });
		}
		if (event.type == sf::Event::KeyReleased) {
			if (event.key.code == sf::Keyboard::Unknown)
				continue;

			keyJustReleased[event.key.code] = true;
			keyPressed[event.key.code] = false;
			
			EventManager::fire("keyReleased", { event.key.code });
		}
		if (event.type == sf::Event::MouseButtonPressed) {
			mouseJustPressed[event.mouseButton.button] = true;
			mousePressed[event.mouseButton.button] = true;
			
			EventManager::fire("mousePressed", { event.mouseButton.button });
		}
		if (event.type == sf::Event::MouseButtonReleased) {
			mouseJustReleased[event.mouseButton.button] = true;
			mousePressed[event.mouseButton.button] = false;
			
			EventManager::fire("mouseReleased", { event.mouseButton.button });
		}
		if (event.type == sf::Event::MouseWheelScrolled) {
			wheelDelta = event.mouseWheelScroll.delta;
		}
	}
	for (i32 i = 0u; i < sf::Keyboard::KeyCount; ++i) {
		if (keyPressed[i])
			EventManager::fire("keyPress", { i });
	}
	for (i32 i = 0u; i < sf::Mouse::ButtonCount; ++i) {
		if (mousePressed[i])
			EventManager::fire("mousePress", { i });
	}

	oldMouseScreePos = mouseScreenPos;
	mouseScreenPos = sf::Mouse::getPosition(window);
}
