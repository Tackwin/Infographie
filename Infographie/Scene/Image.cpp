#include "Image.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

size_t Image::Total_N = 0;

Image::Image(sf::Texture& texture) noexcept {
	Total_N++;
	n = Total_N;
	sprite.setTexture(texture);
}

void Image::update(float dt) noexcept {
	Widget::update(dt);
	if (!is_visible()) return;
	if (!open) return;

	ImGui::Begin(std::to_string(n).c_str(), &open);
	ImGui::Image(sprite);
	ImGui::End();
}
