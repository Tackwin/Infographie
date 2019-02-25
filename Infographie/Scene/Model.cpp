#include <typeinfo>
#include <GL/glew.h>
#include "Common.hpp"

#include "Model.hpp"

#include "Math/Matrix.hpp"

#include "Utils/Logs.hpp"

#include "Camera.hpp"

#include "Managers/AssetsManager.hpp"

size_t Model::Total_N = 0;
constexpr auto Plain_Cube_Boundingbox_Texture_Key = "plain_cube_boundingbox";

Model::Model(bool without_bounding_box) noexcept {
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
	boundingbox_child->set_object_box(size3);
	boundingbox_child->set_position({ 0, size3.y / 2, 0 });
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

void Model::update(float) noexcept {
	if (!visible) return;
	if (!vertex_array_id) return;
	glBindVertexArray(*vertex_array_id);
	defer{ glBindVertexArray(0); };

	auto& obj_to_use = object_file ? *object_file : object_file_copy;

	if (shader) {
		auto camera = (Camera*)find_parent(typeid(Camera));

		float rotate = 0.f;
		Vector3f light_pos;

		if (debug_values["Rotate"].type() == typeid(float)) {
			rotate = std::any_cast<float>(debug_values["Rotate"]);
		}
		if (debug_values["Light_X"].type() == typeid(float)) {
			light_pos.x = std::any_cast<float>(debug_values["Light_X"]);
		}
		if (debug_values["Light_Y"].type() == typeid(float)) {
			light_pos.y = std::any_cast<float>(debug_values["Light_Y"]);
		}
		if (debug_values["Light_Z"].type() == typeid(float)) {
			light_pos.z = std::any_cast<float>(debug_values["Light_Z"]);
		}

		Matrix4f model = Matrix4f::translation(get_global_position3()) * Matrix4f::rotation({ 0, 1, 0 }, rotate);
		Matrix4f view = camera->get_view_matrix();
		Matrix4f projection = camera->get_projection_matrix();

		glUseProgram(shader->getNativeHandle());
		auto uni = glGetUniformLocation(shader->getNativeHandle(), "model");
		glUniformMatrix4fv(uni, 1, GL_FALSE, &model[0][0]);
		uni = glGetUniformLocation(shader->getNativeHandle(), "view");
		glUniformMatrix4fv(uni, 1, GL_FALSE, &(view)[0][0]);
		uni = glGetUniformLocation(shader->getNativeHandle(), "projection");
		glUniformMatrix4fv(uni, 1, GL_FALSE, &(projection)[0][0]);
		uni = glGetUniformLocation(shader->getNativeHandle(), "light_pos");
		glUniform3f(uni, UNROLL_3(light_pos));
	}
	defer{ if (shader) glUseProgram(0); };
	if (texture) {
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle());
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		if (shader)
			glUniform1i(glGetUniformLocation(shader->getNativeHandle(), "my_texture"), 0);
	}
	defer{ if (texture) glBindTexture(GL_TEXTURE_2D, 0); };

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
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, obj_to_use.vertices.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
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
	size3 = o.max - o.min;

	if (boundingbox_child) boundingbox_child->set_object_box(size3);
}

void Model::set_object_box(Vector3f size) noexcept {
	object_file_copy = Object_File::cube(size);
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

	size3 = o.max - o.min;
	set_position({ 0, size3.y / 2, 0 });
}


void Model::set_texture(const sf::Texture& t) noexcept {
	texture = &t;
}

void Model::set_shader(sf::Shader& s) noexcept {
	shader = &s;
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
