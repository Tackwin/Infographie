#include "Camera.hpp"
#include <SFML/OpenGl.hpp>

#include "Managers/InputsManager.hpp"

#include "Model.hpp"

#include "Window.hpp"

Camera::Camera() noexcept {
	pos3 = { 0, 0, 0 };
}

void Camera::render_from(Widget3* root) noexcept {
	render_root = root;
}
Widget3* Camera::get_render_root() const noexcept {
	return render_root;
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


	if (IM::isMouseJustPressed(sf::Mouse::Right)) {
		Vector3f ray_origin = pos3;
		Vector3f ray = {
			1.0f - (2.0f * IM::getMouseScreenPos().x) / Window_Info.size.x,
			(2.0f * IM::getMouseScreenPos().y) / Window_Info.size.y - 1.0f,
			1
		};
		Vector4f ray_clip = { ray.x, ray.y, -1, 1 };
		auto ray_eye = (*projection.invert()) * ray_clip;
		ray_eye.z = -1;
		ray_eye.w = 0;
		auto ray4 = (*view.invert()) * ray_eye;
		ray = Vector3f{ ray4.x, ray4.y, ray.z }.normalize();

		Widget3* selected{ nullptr };
		Vector3f intersection;

		std::vector<Widget*> all_child;
		all_child.push_back(render_root);

		while (!all_child.empty()) {
			auto& w = all_child.back();
			all_child.pop_back();

			if (auto w3 = dynamic_cast<Widget3*>(w); w3) {
				if (auto opt = w3->is_selected(ray_origin, ray)) {
					if (
						!selected ||
						(intersection - ray_origin).length2() < (*opt - ray_origin).length2()
					) {
						selected = w3;
						intersection = *opt;
					}
					continue;
				}
			}

			for (auto& c : w->get_childs()) {
				all_child.push_back(c.get());
			}
		}

		// if we are not multi selcting we take away the focus.
		if (!IM::is_one_of_pressed({ sf::Keyboard::LShift, sf::Keyboard::RShift })) {
			all_child.clear();
			all_child.push_back(render_root);
			while (!all_child.empty()) {
				auto& w = all_child.back();
				all_child.pop_back();

				w->set_focus(false);
				w->lock_focus(false);

				for (auto& c : w->get_childs()) all_child.push_back(c.get());
			}
		}
		if (selected) {
			selected->set_focus(true);
			selected->lock_focus(true);
		}
	}

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
	projection = Matrix4f::perspective(fov, ratio, f, n);
}

const Matrix4f& Camera::get_view_matrix() noexcept {
	return view;
}
const Matrix4f& Camera::get_projection_matrix() noexcept {
	return projection;
}

