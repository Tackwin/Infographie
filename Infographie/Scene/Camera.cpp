#include "Camera.hpp"
#include <SFML/OpenGl.hpp>

#include "Managers/InputsManager.hpp"

void Camera::update(float dt) noexcept {

	if (IM::isKeyPressed(sf::Keyboard::Q)) {
		auto right = look_dir.cross({ 0, 1, 0 });
		pos3 -= dt * right;
	}
	if (IM::isKeyPressed(sf::Keyboard::D)) {
		auto right = look_dir.cross({ 0, 1, 0 });
		pos3 += dt * right;
	}
	if (IM::isKeyPressed(sf::Keyboard::Z)) {
		auto right = Vector3f{ 0, 1, 0 }.cross(look_dir);
		auto up = look_dir.cross(right);
		pos3 += dt * up;
	}
	if (IM::isKeyPressed(sf::Keyboard::S)) {
		auto right = Vector3f{ 0, 1, 0 }.cross(look_dir);
		auto up = look_dir.cross(right);
		pos3 -= dt * up;
	}

}

void Camera::set_gl_view() noexcept {
}


void Camera::look_at(Vector3f p, float dist) noexcept {

}

