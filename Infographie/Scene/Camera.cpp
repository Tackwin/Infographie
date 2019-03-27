#include "Camera.hpp"
#include <SFML/OpenGl.hpp>

#include "Managers/InputsManager.hpp"
#include "Managers/AssetsManager.hpp"
#include "Math/algorithms.hpp"

#include "UI/Illumination.hpp"

#include "Model.hpp"

#include "Window.hpp"

Camera::Camera() noexcept : g_buffer(Window_Info.size) {
	pos3 = { 0, 0, 0 };
}

void Camera::render_from(Widget3* root) noexcept {
	render_root = root;
}
Widget3* Camera::get_render_root() const noexcept {
	return render_root;
}

void Camera::render(sf::RenderTarget& target) noexcept {
	glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
	compute_view();

	if (!render_root) return;

	Window_Info.active_camera = this;

	// Geometry phase
	g_buffer.set_active();
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_root->propagate_opengl_render();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// the lighting phase
	g_buffer.set_active_texture();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto& shader = AM->get_shader("Deferred_Light");

	shader.setUniform("gPosition", 0);
	shader.setUniform("gNormal", 1);
	shader.setUniform("gAlbedoSpec", 2);
	shader.setUniform("view_pos", sf::Vector3f{ UNROLL_3(get_global_position3()) });
	sf::Shader::bind(&shader);

	g_buffer.render_quad();

	// The last phase

	g_buffer.copy_depth();

	render_root->propagate_last_opengl_render();
}


void Camera::update(float dt) noexcept {
	if (!input_active) return;

	if (debug_values["Camera_Speed"].type() == typeid(float)) {
		speed = std::any_cast<float>(debug_values["Camera_Speed"]);
	}

	Vector3f min;
	Vector3f max;
	bool tracking{ false };
	for (auto& id : ids_to_lock) {
		auto it = render_root->find_child(id);
		if (!it) continue;
		if (auto p = dynamic_cast<Widget3*>(it); p) {
			if (id == ids_to_lock.front()) {
				min = p->get_global_position3();
				max = p->get_global_position3();
				tracking = true;
			}
			else {
				auto x = p->get_global_position3();

				min.x = min.x > x.x ? x.x : min.x;
				min.y = min.y > x.y ? x.y : min.y;
				min.z = min.z > x.z ? x.z : min.z;

				max.x = max.x < x.x ? x.x : max.x;
				max.y = max.y < x.y ? x.y : max.y;
				max.z = max.z < x.z ? x.z : max.z;
			}
		}
	}

	if (IM::isKeyPressed(sf::Keyboard::Q)) {
		pos3 -= speed * dt * right;
	}
	if (IM::isKeyPressed(sf::Keyboard::D)) {
		pos3 += speed * dt * right;
	}
	if (IM::isKeyPressed(sf::Keyboard::Z)) {
		pos3 += speed * dt * up;
	}
	if (IM::isKeyPressed(sf::Keyboard::S)) {
		pos3 -= speed * dt * up;
	}
	if (IM::isKeyPressed(sf::Keyboard::LShift)) {
		pos3 += speed * dt * look_dir;
	}
	if (IM::isKeyPressed(sf::Keyboard::LControl)) {
		pos3 -= speed * dt * look_dir;
	}
	if (!tracking && IM::isMousePressed(sf::Mouse::Middle)) {
		auto mouse_dt = IM::getMouseScreenDelta().normalize();

		yaw += 0.3f * speed * mouse_dt.x * dt;
		pitch += 0.3f * speed * mouse_dt.y * dt;

		look_dir.x = -sin(yaw) * cos(pitch);
		look_dir.y = -sin(pitch);
		look_dir.z = -cos(yaw) * cos(pitch);

		right.x = -cos(yaw);
		right.y = 0.0;
		right.z = sin(yaw);

		up = look_dir.cross(right);

		look_dir = look_dir.normalize();
		right = right.normalize();
		up = up.normalize();
	}
	else if (tracking) {
		look_at((max + min) / 2, up);
	}
	if (IM::isMouseJustPressed(sf::Mouse::Right)) select_ray_cast();
}

void Camera::select_ray_cast() noexcept {
	// >TP1
			// i don't know why this code is buggy ?? the ray projection is either off or it's
			// the render ? There seem to be an off by a-factor-error.
			// When the camera is straight looking and the object is at the center, it's ok
			// When the camera is straight looking and the object is at an offset, it's not but on
			//		the same trajectory
			// and it's the same if the camera is at an angle.
			// so just use the ui to select objects

		// >TP2
		// Turn out my view and proection matrix were wrong all along, they needed to be
		// transposed, damn you linear algebra and your non obvious graphical bug argh....

		// My god i was doing	Vector3f{ ray4.x, ray4.y, ray.z } instead of
		//						Vector3f{ ray4.x, ray4.y, ray4.z } ... (L+13)

	auto ray_origin = pos3;
	auto ray = get_ray_from_graphic_matrices(get_mouse_viewport(), projection, view);

	Widget3 * selected{ nullptr };
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

void Camera::set_viewport(Rectangle2u rec) noexcept {
	viewport = rec;
}

void Camera::look_at(Vector3f target, Vector3f u) noexcept {
	look_dir = (target - get_global_position3()).normalize();
	right = look_dir.cross(u).normalize();
	up = right.cross(look_dir).normalize();
}

void Camera::set_perspective(float fov, float ratio, float f, float n) noexcept {
	projection = Matrix4f::perspective(fov, ratio, f, n);
}

void Camera::set_orthographic(float scale, float ratio, float f, float n) noexcept {
	projection = Matrix4f::orthographic(scale, ratio, f, n);
}

const Matrix4f& Camera::get_view_matrix() noexcept {
	return view;
}
const Matrix4f& Camera::get_projection_matrix() noexcept {
	return projection;
}

void Camera::compute_view() noexcept {
	auto xaxis = right;// The "right" vector.
	auto yaxis = up;     // The "up" vector.
	auto zaxis = -1 * look_dir;    // The "forward" vector.

	// Create a 4x4 view matrix from the right, up, forward and eye position vectors
	float mat_comp[]{
		xaxis.x,            yaxis.x,            zaxis.x,       0.f,
		xaxis.y,            yaxis.y,            zaxis.y,       0.f,
		xaxis.z,            yaxis.z,            zaxis.z,       0.f,
		0,                        0,                  0,       1.f
	};

	view = { mat_comp };
	view = view * Matrix4f::translation(-1 * get_global_position3());
}

void Camera::set_speed(float s) noexcept {
	speed = s;
}

void Camera::lock(std::vector<Uuid_t> ids) noexcept {
	ids_to_lock = std::move(ids);
}

Vector2f Camera::get_mouse_viewport() const noexcept {
	return {
		(2.f * (IM::getMouseScreenPos().x - viewport.x)) / viewport.w - 1.f,
		1.f - (2.f * IM::getMouseScreenPos().y - viewport.y) / viewport.h
	};
}

bool Camera::is_input_active() const noexcept {
	return input_active;
}

void Camera::set_input_active(bool v) noexcept {
	input_active = v;
}
