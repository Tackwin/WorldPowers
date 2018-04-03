#include "MapScene.hpp"

#include "./../Managers/InputsManager.hpp"
#include "./../Managers/MemoryManager.hpp"
#include "./../Map/Map.hpp"

#include "imgui.h"

MapScene::MapScene(Game* g, Map* m) : Scene(g), _map(m) {
}

MapScene::~MapScene() {

}

void MapScene::input() {
	if (IM::isKeyPressed(sf::Keyboard::LControl) && IM::isMousePressed(sf::Mouse::Left)) {
		_map->getView().move(-1.f * IM::getDtMouseScreenPos() * _viewScale);
	}
	if (IM::getWheelDelta() != 0) {
		_viewScale *= std::powf(2, -IM::getWheelDelta());
		_map->getView().zoom(std::powf(2, -IM::getWheelDelta()));
	}
}

void MapScene::update(dt_t dt) {

	_map->update(dt);

	updateImGui();
}

void MapScene::updateImGui() {
	ImGui::Begin("Map");
	if (ImGui::CollapsingHeader("Generate")) {
		ImGui::SliderFloat("poisson r", &_imgui_data.poisson_r, 1.f, 100.f, "%.3f", 2.f);
		if (ImGui::Button("Go")) {
			_map->build(_imgui_data.poisson_r);
		}
	}
	if (ImGui::CollapsingHeader("Render flags")) {
		ImGui::Checkbox("Elevation", &_imgui_data.render_elevation);
		ImGui::Checkbox("Downhill", &_imgui_data.render_downhilness);
	}
	ImGui::End();

	ImGui::Begin("Mem");
	ImGui::LabelText("size", "%u", MM::I().get_buffer_size());
	ImGui::LabelText("free", "%u", MM::I().get_free_size());
	ImGui::End();

	ImGui::Begin("Info");
	ImGui::LabelText("Elevation",
		"E: %f",
		_map->getElevation(_map->screenToMap((Vector2d)IM::getMouseScreenPos()))
	);
	ImGui::End();
}


void MapScene::render(sf::RenderTarget& target) {
	auto oldView = target.getView();

	u08 renderFlags = Map::RenderFlags::VOID;
	if (_imgui_data.render_elevation)
		renderFlags |= Map::RenderFlags::ELEVATION;
	if (_imgui_data.render_downhilness)
		renderFlags |= Map::RenderFlags::DOWNHILLNESS;

	_map->render(target, (Map::RenderFlags)renderFlags);

	target.setView(oldView);
}