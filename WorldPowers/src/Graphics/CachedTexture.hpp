#pragma once

#include <SFML/Graphics.hpp>

class CachedTexture {

public:

	void setImage(const sf::Image& img) noexcept;

	constexpr bool isCached() const noexcept;

private:

	bool _cached{ false };
	sf::Texture _texture{};

};