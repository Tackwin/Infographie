#include <GL/glew.h>
#include "Common.hpp"

#include "Model.hpp"

#include "Math/Matrix.hpp"

#include "Utils/Logs.hpp"

size_t Model::Total_N = 0;
Model::Model() noexcept {
	Total_N++;
	n = Total_N;
}

Model::~Model() noexcept {
	if (vertex_array_id) {
		glDeleteBuffers(1, &*vertex_buffer_id);
		glDeleteBuffers(1, &*uv_buffer_id);
		glDeleteVertexArrays(1, &*vertex_array_id);
	}
}

void Model::update(float) noexcept {
	if (!vertex_array_id) return;
	glBindVertexArray(*vertex_array_id);
	defer{ glBindVertexArray(0); };

	if (shader) {
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
		Matrix4f model = Matrix4f::rotation({ 0, 1, 0 }, rotate);
			// Matrix4f::translation(pos3);
			// Matrix4f::rotation({ 1, 0, 0 }, 0) *
			// Matrix4f::scale(1);
		Matrix4f p = Matrix4f::scale(0.12f);

		glUseProgram(shader->getNativeHandle());
		check_gl_error();
		auto uni = glGetUniformLocation(shader->getNativeHandle(), "model");
		check_gl_error();
		glUniformMatrix4fv(uni, 1, GL_FALSE, &model[0][0]);
		uni = glGetUniformLocation(shader->getNativeHandle(), "view");
		check_gl_error();
		glUniformMatrix4fv(uni, 1, GL_FALSE, &(*View_Matrix)[0][0]);
		uni = glGetUniformLocation(shader->getNativeHandle(), "projection");
		check_gl_error();
		glUniformMatrix4fv(uni, 1, GL_FALSE, &(p)[0][0]);


		uni = glGetUniformLocation(shader->getNativeHandle(), "light_pos");
		check_gl_error();
		glUniform3f(uni, UNROLL_3(light_pos));


		check_gl_error();
	}
	defer{ if (shader) glUseProgram(0); };
	if (texture) {
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0); check_gl_error();
		glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle()); check_gl_error();
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		if (shader)
			glUniform1i(glGetUniformLocation(shader->getNativeHandle(), "my_texture"), 0);
	}
	defer{ if (texture) glBindTexture(GL_TEXTURE_2D, 0); };

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	check_gl_error();
	glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer_id);
	defer{ glBindBuffer(GL_ARRAY_BUFFER, 0); };
	check_gl_error();
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	check_gl_error();

	glBindBuffer(GL_ARRAY_BUFFER, *uv_buffer_id);
	defer{ glBindBuffer(GL_ARRAY_BUFFER, 0); };
	check_gl_error();
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	check_gl_error();

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
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
	glDrawArrays(GL_TRIANGLES, 0, object_file->vertices.size());
	check_gl_error();

	glDisableVertexAttribArray(0);
	check_gl_error();
	glDisableVertexAttribArray(1);
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

