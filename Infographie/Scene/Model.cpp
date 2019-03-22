#include <typeinfo>
#include <GL/glew.h>
#include "Common.hpp"

#include "Model.hpp"

#include "Math/Matrix.hpp"

#include "Utils/Logs.hpp"
#include "Utils/TimeInfo.hpp"

#include "Camera.hpp"
#include "Window.hpp"

#include "Managers/AssetsManager.hpp"
#include "Managers/InputsManager.hpp"
#include "Math/algorithms.hpp"

#include "imgui/imgui.h"

size_t Model::Total_N = 0;
constexpr auto Plain_Cube_Boundingbox_Texture_Key = "plain_cube_boundingbox";

Model::Model(bool w) noexcept : without_bounding_box(w) {
	Total_N++;
	n = Total_N;

	if (without_bounding_box) return;
	if (!AM->have_texture(Plain_Cube_Boundingbox_Texture_Key)) {
		sf::Image plain;
		plain.create(1, 1);
		plain.setPixel(0, 0, sf::Color{ 255, 30, 30, 100 });

		auto& texture_plain = AM->create_texture(Plain_Cube_Boundingbox_Texture_Key);
		texture_plain.loadFromImage(plain);
	}

	boundingbox_child = make_child<Model>(true);
	boundingbox_child->set_object_copy(Object_File::cube(size3));
	boundingbox_child->set_texture(AM->get_texture(Plain_Cube_Boundingbox_Texture_Key));
	boundingbox_child->set_shader(AM->get_shader("Simple"));
	boundingbox_child->set_visible(false);
}

Model::~Model() noexcept {
	if (vertex_array_id) {
		glDeleteBuffers(1, &*vertex_buffer_id);
		glDeleteBuffers(1, &*uv_buffer_id);
		glDeleteVertexArrays(1, &*vertex_array_id);
	}
}

void Model::opengl_render() noexcept {
	if (!visible) return;
	if (!vertex_array_id) return;

	glBindVertexArray(*vertex_array_id);
	defer{ glBindVertexArray(0); };

	auto& obj_to_use = object_file ? *object_file : object_file_copy;

	if (shader || select_shader) {
		auto s = shader;
		if (is_focus() && select_shader) s = select_shader;

		Matrix4f model =
			Matrix4f::translation(get_global_position3()) *
			Matrix4f::rotation(rotation3) *
			Matrix4f::scale(scaling);
		Matrix4f view = Window_Info.active_camera->get_view_matrix();
		Matrix4f proj = Window_Info.active_camera->get_projection_matrix();
		Matrix4f view_wo_pos = view;

		view_wo_pos[{3, 0}] = 0;
		view_wo_pos[{3, 1}] = 0;
		view_wo_pos[{3, 2}] = 0;
		view_wo_pos[{3, 3}] = 1;

		auto handle = s->getNativeHandle();

		glUseProgram(handle);
		glUniformMatrix4fv(glGetUniformLocation(handle, "model"), 1, GL_FALSE, (float*)&model);
		glUniformMatrix4fv(glGetUniformLocation(handle, "view"), 1, GL_FALSE, (float*)&view);
		glUniformMatrix4fv(
			glGetUniformLocation(handle, "view_wo_pos"), 1, GL_FALSE, (float*)&view_wo_pos
		);
		glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), 1, GL_FALSE, (float*)&proj);
		glUniform1i(glGetUniformLocation(handle, "texture_main"), 0);
		glUniform1i(glGetUniformLocation(handle, "texture_alpha"), 1);
		glUniform1i(glGetUniformLocation(handle, "use_alpha"), 0);
		glUniform1f(glGetUniformLocation(handle, "alpha_tolerance"), Alpha_Tolerance);

		if (s == select_shader) {
			float a = (get_milliseconds_epoch() % 500000) / 1000.f;
			glUniform1f(glGetUniformLocation(handle, "time"), a);
		}
		if (texture) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle());
		}
		if (alpha_texture) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, alpha_texture->getNativeHandle());
			glUniform1i(glGetUniformLocation(handle, "use_alpha"), 1);
		}
	}
	defer{
		if (shader) glUseProgram(0);
		if (texture) glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
	};

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer_id);
	defer{ glBindBuffer(GL_ARRAY_BUFFER, 0); };
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	glBindBuffer(GL_ARRAY_BUFFER, *uv_buffer_id);
	defer{ glBindBuffer(GL_ARRAY_BUFFER, 0); };
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	glBindBuffer(GL_ARRAY_BUFFER, *normal_buffer_id);
	defer{ glBindBuffer(GL_ARRAY_BUFFER, 0); };
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	check_gl_error();
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, obj_to_use.vertices.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	check_gl_error();
}

void Model::set_object(const Object_File& o) noexcept {
	if (vertex_array_id) {
		glDeleteBuffers(1, &*vertex_buffer_id);
		check_gl_error();
		glDeleteBuffers(1, &*uv_buffer_id);
		check_gl_error();
		glDeleteVertexArrays(1, &*vertex_array_id);
		check_gl_error();
	}

	vertex_array_id = 0;
	uv_buffer_id = 0;
	vertex_buffer_id = 0;
	normal_buffer_id = 0;
	glGenVertexArrays(1, &*vertex_array_id);
	check_gl_error();
	glBindVertexArray(*vertex_array_id);
	defer{ glBindVertexArray(0); };
	check_gl_error();

	glGenBuffers(1, &*vertex_buffer_id);
	check_gl_error();
	glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer_id);
	defer{ glBindBuffer(GL_ARRAY_BUFFER, 0); };
	check_gl_error();
	glBufferData(
		GL_ARRAY_BUFFER,
		o.vertices.size() * sizeof(Vector3f),
		o.vertices.data(),
		GL_STATIC_DRAW
	);
	check_gl_error();

	glGenBuffers(1, &*uv_buffer_id);
	check_gl_error();
	glBindBuffer(GL_ARRAY_BUFFER, *uv_buffer_id);
	defer{ glBindBuffer(GL_ARRAY_BUFFER, 0); };
	check_gl_error();
	glBufferData(
		GL_ARRAY_BUFFER,
		o.uvs.size() * sizeof(Vector2f),
		o.uvs.data(),
		GL_STATIC_DRAW
	);
	check_gl_error();

	glGenBuffers(1, &*normal_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, *normal_buffer_id);
	glBufferData(
		GL_ARRAY_BUFFER,
		o.normals.size() * sizeof(Vector3f),
		o.normals.data(),
		GL_STATIC_DRAW
	);

	object_file = &o;
	set_size(o.max - o.min);
}

void Model::set_object_copy(const Object_File& obj) noexcept {
	object_file_copy = obj;
	auto& o = object_file_copy;

	if (vertex_array_id) {
		glDeleteBuffers(1, &*vertex_buffer_id);
		check_gl_error();
		glDeleteBuffers(1, &*uv_buffer_id);
		check_gl_error();
		glDeleteVertexArrays(1, &*vertex_array_id);
		check_gl_error();
	}

	vertex_array_id = 0;
	uv_buffer_id = 0;
	vertex_buffer_id = 0;
	normal_buffer_id = 0;
	glGenVertexArrays(1, &*vertex_array_id);
	check_gl_error();
	glBindVertexArray(*vertex_array_id);
	defer{ glBindVertexArray(0); };
	check_gl_error();

	glGenBuffers(1, &*vertex_buffer_id);
	check_gl_error();
	glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer_id);
	defer{ glBindBuffer(GL_ARRAY_BUFFER, 0); };
	check_gl_error();
	glBufferData(
		GL_ARRAY_BUFFER,
		o.vertices.size() * sizeof(Vector3f),
		o.vertices.data(),
		GL_STATIC_DRAW
	);
	check_gl_error();

	glGenBuffers(1, &*uv_buffer_id);
	check_gl_error();
	glBindBuffer(GL_ARRAY_BUFFER, *uv_buffer_id);
	defer{ glBindBuffer(GL_ARRAY_BUFFER, 0); };
	check_gl_error();
	glBufferData(
		GL_ARRAY_BUFFER,
		o.uvs.size() * sizeof(Vector2f),
		o.uvs.data(),
		GL_STATIC_DRAW
	);
	check_gl_error();

	glGenBuffers(1, &*normal_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, *normal_buffer_id);
	glBufferData(
		GL_ARRAY_BUFFER,
		o.normals.size() * sizeof(Vector3f),
		o.normals.data(),
		GL_STATIC_DRAW
	);

	set_size(o.max - o.min);
}



void Model::set_texture(const sf::Texture& t) noexcept {
	texture = &t;
}
void Model::set_alpha_texture(const sf::Texture& t) noexcept {
	alpha_texture = &t;
}


void Model::set_shader(sf::Shader& s) noexcept {
	shader = &s;
}
void Model::set_select_shader(sf::Shader& s) noexcept {
	select_shader = &s;
}


const sf::Texture* Model::get_texture() const noexcept {
	return texture;
}

size_t Model::get_n() const noexcept {
	return n;
}

bool Model::does_render_checkbox() noexcept {
	return render_checkbox;
}
void Model::set_render_checkbox(bool v) noexcept {
	render_checkbox = v;
	boundingbox_child->set_visible(v);
}

float Model::get_picking_sphere_radius() const noexcept {
	return picking_sphere_radius;
}

void Model::set_size(Vector3f s) noexcept {
	Widget3::set_size(s);
	size3 = s;
	picking_sphere_radius = std::powf(3 * s.x * s.y * s.z / (4 * PIf), 1 / 3.f);
	if (boundingbox_child && !without_bounding_box) {
		auto& o = (object_file ? *object_file : object_file_copy);

		boundingbox_child->set_object_copy(Object_File::cube(size3));
		boundingbox_child->set_position((o.min + o.max) / 2);
	}
}

std::optional<Vector3f> Model::is_selected(Vector3f ray_origin, Vector3f ray) const noexcept {
	Widget3::is_selected(ray_origin, ray);
	if (!is_visible()) return std::nullopt;

	auto sphere_center = get_global_position3();
	return Vector3f::ray_intersect_sphere(sphere_center, picking_sphere_radius, ray_origin, ray);
/*	std::optional<Vector3f> intersection;

	for (size_t i = 0; i < o.vertices.size(); i += 3) {
		auto intersection_vec = Vector3f::intersect_tirangle(
			ray_origin, ray, o.vertices[i + 0], o.vertices[i + 1], o.vertices[i + 2]
		);
		if (!intersection_vec) continue;
		if (
			!intersection ||
			(*intersection_vec - ray_origin).length2() < (*intersection - ray_origin).length2()
		) {
			intersection = intersection_vec;
		}
	}
	return intersection;
*/}

void Model::set_focus(bool v) noexcept {
	Widget::set_focus(v);

	if (!selectable) return;

	if (!picker && v) push_picker();
	if (picker && !v) pop_picker();
}

void Model::set_scaling(Vector3f s) noexcept {
	scaling = s;
	if (boundingbox_child) {
		boundingbox_child->set_scaling(s);
	}
}
Vector3f Model::get_scaling() const noexcept {
	return scaling;
}

Model::Picker::Picker() noexcept {
	xy_plan = std::make_unique<Model>(true);
	yz_plan = std::make_unique<Model>(true);
	zx_plan = std::make_unique<Model>(true);

	xy_plan->set_object_copy(Object_File::cube({ 1, 1, 0.1f }));
	yz_plan->set_object_copy(Object_File::cube({ 0.1f, 1, 1 }));
	zx_plan->set_object_copy(Object_File::cube({ 1, 0.1f, 1 }));

	xy_plan->set_texture(AM->get_texture(Plain_Cube_Boundingbox_Texture_Key));
	yz_plan->set_texture(AM->get_texture(Plain_Cube_Boundingbox_Texture_Key));
	zx_plan->set_texture(AM->get_texture(Plain_Cube_Boundingbox_Texture_Key));

	xy_plan->set_shader(AM->get_shader("Simple"));
	yz_plan->set_shader(AM->get_shader("Simple"));
	zx_plan->set_shader(AM->get_shader("Simple"));

	xy_plan->set_position({ 0, 0, 0.6f });
	yz_plan->set_position({ 0.6f, 0, 0 });
	zx_plan->set_position({ 0, -0.6f, 0 });

	xy_plan->set_selectable(false);
	yz_plan->set_selectable(false);
	zx_plan->set_selectable(false);
}

void Model::Picker::update(float) noexcept {
	if (IM::isMouseJustReleased(sf::Mouse::Left)) {
		selected_plan = Plan_List::None;
		return;
	}


	Ray3f ray;
	ray.pos = Window_Info.active_camera->get_global_position3();
	ray.dir = get_ray_from_graphic_matrices(
		{
			(2.f * IM::getMouseScreenPos().x) / Window_Info.size.x - 1.f,
			1.f - (2.f * IM::getMouseScreenPos().y) / Window_Info.size.y
		},
		Window_Info.active_camera->get_projection_matrix(),
		Window_Info.active_camera->get_view_matrix()
	);
	if (IM::isMouseJustPressed(sf::Mouse::Left)) {
		auto p = xy_plan->get_global_position3();
		auto s = xy_plan->get_scaling().hamilton(xy_plan->get_size3());

		auto xy_intersection = ray_plane(
			ray,
			xy_plan->get_global_position3(),
			{0, 0, 1}
		);
		if (xy_intersection &&
			p.x - s.x / 2.f < xy_intersection->x && xy_intersection->x < p.x + s.x / 2.f &&
			p.y - s.y / 2.f < xy_intersection->y && xy_intersection->y < p.y + s.y / 2.f
		) {
			selected_plan = Plan_List::XY;
		}

		p = yz_plan->get_global_position3();
		s = yz_plan->get_scaling().hamilton(yz_plan->get_size3());

		auto yz_intersection = ray_plane(
			ray,
			yz_plan->get_global_position3(),
			{ 1, 0, 0 }
		);
		if (yz_intersection &&
			p.y - s.y / 2.f < yz_intersection->y && yz_intersection->y < p.y + s.y / 2.f &&
			p.z - s.z / 2.f < yz_intersection->z && yz_intersection->z < p.z + s.z / 2.f
		) {
			selected_plan = Plan_List::YZ;
		}

		p = zx_plan->get_global_position3();
		s = zx_plan->get_scaling().hamilton(zx_plan->get_size3());

		auto zx_intersection = ray_plane(
			ray,
			zx_plan->get_global_position3(),
			{ 0, 1, 0 }
		);
		if (zx_intersection &&
			p.z - s.z / 2.f < zx_intersection->z && zx_intersection->z < p.z + s.z / 2.f &&
			p.x - s.x / 2.f < zx_intersection->x && zx_intersection->x < p.x + s.x / 2.f
		) {
			selected_plan = Plan_List::ZX;
		}

		initial_pos = get_global_position3();
	}

	switch (selected_plan) {
	case Plan_List::XY: {
		float t = (initial_pos.z - ray.pos.z) / ray.dir.z;
		((Model*)parent)->set_global_position(ray.pos + t * ray.dir);
		break;
	}
	case Plan_List::YZ: {
		float t = (initial_pos.x - ray.pos.x) / ray.dir.x;
		((Model*)parent)->set_global_position(ray.pos + t * ray.dir);
		break;
	}
	case Plan_List::ZX: {
		float t = (initial_pos.y - ray.pos.y) / ray.dir.y;
		((Model*)parent)->set_global_position(ray.pos + t * ray.dir);
		break;
	}
	}


}

void Model::Picker::last_opengl_render() noexcept {
	ImGui::Text(
		"x: %3.3f, y: %3.3f, z: %3.3f",
		get_global_position3().x,
		get_global_position3().y,
		get_global_position3().z
	);

	auto dist =
		(Window_Info.active_camera->get_global_position3() - get_global_position3()).length();

	// It's crazy we can make constant screen size just by scaling world woordinate by camera
	// distance.
	glClear(GL_DEPTH_BUFFER_BIT);

	Vector3f scale{ dist / 20, dist / 20, dist / 20 };

	xy_plan->set_position(get_global_position3() + scale.x * Vector3f{ 0, 0, 0.6f });
	yz_plan->set_position(get_global_position3() + scale.x * Vector3f{ 0.6f, 0, 0 });
	zx_plan->set_position(get_global_position3() + scale.x * Vector3f{ 0, -0.6f, 0 });

	xy_plan->set_scaling(scale);
	yz_plan->set_scaling(scale);
	zx_plan->set_scaling(scale);

	xy_plan->opengl_render();
	yz_plan->opengl_render();
	zx_plan->opengl_render();
}


void Model::toggle_picker() noexcept {
	if (picker) pop_picker();
	else push_picker();
}


void Model::push_picker() noexcept {
	assert(!picker);
	picker = new Picker();
	add_child(picker, -1);
}

void Model::pop_picker() noexcept {
	assert(picker);
	kill_direct_child(picker->get_uuid());
	picker = nullptr;
}

void Model::set_selectable(bool v) noexcept {
	selectable = v;
}
