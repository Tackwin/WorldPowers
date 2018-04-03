#include "CachedTexture.hpp"

using CH = CachedTexture;

void CH::setImage(const sf::Image& img) noexcept {
	_texture.loadFromImage(img);
	_cached = true;
}

constexpr bool CH::isCached() const noexcept {
	return _cached;
}

