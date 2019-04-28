#include "Camera.hpp"
#include <SFML/OpenGl.hpp>

#include "Managers/InputsManager.hpp"
#include "Managers/AssetsManager.hpp"
#include "Math/algorithms.hpp"

#include "UI/Illumination.hpp"

#include "Model.hpp"

#include "Window.hpp"

#include "imgui/imgui.h"

std::vector<Vector3f> generate_ssao_samples(size_t n) noexcept;
uint32_t get_noise_texture() noexcept;

Camera::Camera() noexcept :
	g_buffer(Window_Info.size), hdr_buffer(Window_Info.size), ssao_buffer(Window_Info.size)
{
	pos3 = { 0, 0, 0 };
}

void Camera::render_from(Widget3* root) noexcept {
	render_root = root;
}
Widget3* Camera::get_render_root() const noexcept {
	return render_root;
}

void Camera::render(Texture_Buffer& target) noexcept {
	static int Show_Debug{ 0 };
	static int Show_Debug_SSAO{ 0 };

	ImGui::DragInt("Show Debug", &Show_Debug, 0.1f, 0, 10);
	ImGui::DragInt("Show Debug SSAO", &Show_Debug_SSAO, 0.1f, 0, 10);

	glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
	compute_view();

	if (!render_root) return;

	Window_Info.active_camera = this;
	auto active_cubemap = find_active_cubemap();

	// Geometry phase
	g_buffer.set_active();
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_root->propagate_opengl_render();
	
	// ssao phase
	// (Not the blur)
	auto samples = generate_ssao_samples(ray_tracing_settings.kernel_sample_count);
	auto noise_texture = get_noise_texture();
	auto& ssao_shader = AM->get_shader("SSAO");
	auto& ssao_shader_blur = AM->get_shader("SSAO_Blur");

	ssao_buffer.set_active_ssao();
	g_buffer.set_active_texture();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// the first 4 are used by the geometry buffer.
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, noise_texture);

	sf::Shader::bind(&ssao_shader);
	ssao_shader.setUniform("gPosition", 0);
	ssao_shader.setUniform("gNormal", 1);
	ssao_shader.setUniform("gMRA", 3);
	ssao_shader.setUniform("texNoise", 4);
	ssao_shader.setUniform("kernelSize", (int)ray_tracing_settings.kernel_sample_count);
	ssao_shader.setUniform("radius", ray_tracing_settings.ssao_radius);
	ssao_shader.setUniform("bias", ray_tracing_settings.ssao_bias);
	auto noise_scale = (Vector2f)(Window_Info.size / ray_tracing_settings.noise_scale);
	glUniform2fv(
		glGetUniformLocation(ssao_shader.getNativeHandle(), "noiseScale"),
		1,
		(float*)& noise_scale.x
	);
	auto global_pos = get_global_position3();
	glUniform3fv(
		glGetUniformLocation(ssao_shader.getNativeHandle(), "cam_pos"),
		1,
		(float*)& global_pos.x
	);
	ssao_shader_blur.setUniform("texture", 0);
	ssao_shader_blur.setUniform("blur_radius", (int)ray_tracing_settings.blur_radius);
	glUniformMatrix4fv(
		glGetUniformLocation(ssao_shader.getNativeHandle(), "projection"),
		1,
		GL_FALSE,
		(float*)& projection
	);
	glUniformMatrix4fv(
		glGetUniformLocation(ssao_shader.getNativeHandle(), "view"),
		1,
		GL_FALSE,
		(float*)& view
	);
	for (size_t i = 0; i < samples.size(); ++i) {
		auto str = std::string("samples[") + std::to_string(i) + "]";
		auto v = samples[i];

		glUniform3fv(
			glGetUniformLocation(ssao_shader.getNativeHandle(), str.c_str()),
			1,
			(float*)&v.x
		);
	}

	ssao_buffer.render_quad();

	// (Blur)
	ssao_buffer.set_active_blur();
	glClear(GL_COLOR_BUFFER_BIT);
	sf::Shader::bind(&ssao_shader_blur);
	ssao_buffer.set_active_texture_for_blur();
	ssao_buffer.render_quad();

	
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	hdr_buffer.set_active();

	// the lighting phase
	g_buffer.set_active_texture();
	ssao_buffer.set_active_texture(10);

	glClearColor(UNROLL_3(Window_Info.clear_color), 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto& shader_light = AM->get_shader("Deferred_Light");

	shader_light.setUniform("gPosition", 0);
	shader_light.setUniform("gNormal", 1);
	shader_light.setUniform("gAlbedoSpec", 2);
	shader_light.setUniform("gMRA", 3);
	shader_light.setUniform("gSSAO", 10);
	shader_light.setUniform("show_debug", Show_Debug);
	shader_light.setUniform("view_pos", sf::Vector3f{ UNROLL_3(get_global_position3()) });
	shader_light.setUniform("use_ibl", 0);

	if (active_cubemap) {
		shader_light.setUniform("use_ibl", 1);
		shader_light.setUniform("irradianceMap", 7);
		shader_light.setUniform("prefilterMap", 8);
		shader_light.setUniform("brdfLUT", 9);

		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_CUBE_MAP, active_cubemap->get_irradiance_id());
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_CUBE_MAP, active_cubemap->get_prefilter_id());
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, active_cubemap->get_brdf_lut_id());
	}

	sf::Shader::bind(&shader_light);
	g_buffer.render_quad();

	// HDR
	target.set_active();
	//target.setActive(true);
	g_buffer.copy_depth_to(target.get_frame_buffer_id());
	
	hdr_buffer.set_active_texture();
	glClear(GL_COLOR_BUFFER_BIT);

	auto& shader_hdr = AM->get_shader("HDR");

	shader_hdr.setUniform("gamma", gamma);
	shader_hdr.setUniform("exposure", exposure);
	shader_hdr.setUniform("hdr_texture", 0);
	sf::Shader::bind(&shader_hdr);

	hdr_buffer.render_quad();

	// The last phase
	target.set_active();
	//target.setActive(true);
	g_buffer.copy_depth_to(target.get_frame_buffer_id());
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
	cam_far = f;
	cam_near = n;
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

float Camera::get_exposure() const noexcept {
	return exposure;
}

void Camera::set_exposure(float x) noexcept {
	exposure = x;
}

float Camera::get_gamma() const noexcept {
	return gamma;
}

void Camera::set_gamma(float x) noexcept {
	gamma = x;
}

Cube_Map* Camera::find_active_cubemap() const noexcept {
	assert(render_root);

	// For now the best is ust the last as it should never happen to have two cubemaps visible
	// at the same time
	Cube_Map* best = nullptr;
	render_root->for_every_childs([&](Widget* w) {
		if (auto candidate = dynamic_cast<Cube_Map*>(w); candidate && candidate->is_visible())
			best = candidate;
	});

	return best;
}

std::vector<Vector3f> generate_ssao_samples(size_t n) noexcept {
	std::uniform_real_distribution<float> unit(0.f, 1.f);
	std::uniform_real_distribution<float> angle(0.f, PIf);
	std::default_random_engine generator;
	std::vector<Vector3f> result;
	for (unsigned int i = 0; i < n; ++i) {
		float angles[2] = { angle(generator), angle(generator) };
		auto sample = Vector3f::createUnitVector(angles);
		sample *= unit(generator);
		float scale = i / (float)n;
		scale = xstd::lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;

		result.push_back(sample);
	}
	return result;
}

uint32_t get_noise_texture() noexcept {
	static std::optional<uint32_t> noise_texture;
	if (!noise_texture) {

		std::uniform_real_distribution<float> float_dist(-1, 1);
		std::default_random_engine generator(SEED);

		std::vector<Vector3f> noise;

		for (unsigned int i = 0; i < 16; i++) {
			Vector3f noise_vec;
			noise_vec.x = float_dist(generator);
			noise_vec.z = float_dist(generator);
			noise_vec.z = 1;

			noise.push_back(noise_vec);
		}
		
		noise_texture = 0;
		glGenTextures(1, &*noise_texture);
		glBindTexture(GL_TEXTURE_2D, *noise_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &noise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	}
	return *noise_texture;
}
