#include "Camera.hpp"
#include <SFML/OpenGl.hpp>

#include "Managers/InputsManager.hpp"

Camera::Camera() noexcept {
	pos3 = { 0, 0, 0 };
}

void Camera::update(float dt) noexcept {

	if (debug_values["Rotate"].type() == typeid(float)) {
		speed = std::any_cast<float>(debug_values["Camera_Speed"]);
	}

	if (IM::isKeyPressed(sf::Keyboard::Q)) {
		auto right = look_dir.cross({ 0, 1, 0 });
		pos3 -= speed * dt * right;
	}
	if (IM::isKeyPressed(sf::Keyboard::D)) {
		auto right = look_dir.cross({ 0, 1, 0 });
		pos3 += speed * dt * right;
	}
	if (IM::isKeyPressed(sf::Keyboard::Z)) {
		auto right = Vector3f{ 0, 1, 0 }.cross(look_dir);
		auto up = look_dir.cross(right);
		pos3 -= speed * dt * up;
	}
	if (IM::isKeyPressed(sf::Keyboard::S)) {
		auto right = Vector3f{ 0, 1, 0 }.cross(look_dir);
		auto up = look_dir.cross(right);
		pos3 += speed * dt * up;
	}
	if (IM::isKeyPressed(sf::Keyboard::LShift)) {
		pos3 += speed * dt * look_dir;
	}
	if (IM::isKeyPressed(sf::Keyboard::LControl)) {
		pos3 -= speed * dt * look_dir;
	}

	glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
	view = Matrix4f::translation(pos3);

}

void Camera::set_viewport(Rectangle2u rec) noexcept {
	viewport = rec;
}

void Camera::look_at(Vector3f target) noexcept {
	auto zaxis = (pos3 - target).normalize();    // The "forward" vector.
	auto xaxis = Vector3f{ 0, 1, 0 }.cross(zaxis).normalize();// The "right" vector.
	auto yaxis = zaxis.cross(xaxis);     // The "up" vector.

	// Create a 4x4 view matrix from the right, up, forward and eye position vectors
	float mat_comp[]{
		xaxis.x,            yaxis.x,            zaxis.x,       0.f,
		xaxis.y,            yaxis.y,            zaxis.y,       0.f,
		xaxis.z,            yaxis.z,            zaxis.z,       0.f,
		-xaxis.dot(pos3), -yaxis.dot(pos3), -zaxis.dot(pos3),  1.f
	};

	view = { mat_comp };
}

void Camera::set_perspective(float fov, float ratio, float f, float n) noexcept {
	//projection = Matrix4f::identity() / 8;
	projection = Matrix4f::perspective(fov, ratio, f, n);
}

const Matrix4f& Camera::get_view_matrix() noexcept {
	return view;
}
const Matrix4f& Camera::get_projection_matrix() noexcept {
	return projection;
}

