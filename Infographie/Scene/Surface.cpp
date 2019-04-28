#include <GL/glew.h>
#include <algorithm>

#include "Common.hpp"
#include "Camera.hpp"
#include "Window.hpp"

#include "imgui/imgui.h"

#include "Managers/AssetsManager.hpp"

#include "Files/FileFormat.hpp"

#include "Surface.hpp"

#include "Math/Matrix.hpp"

#include "Model.hpp"

// We use hard coded shader because we use tesselation only here and sfml get one again in my way
// as they don't support tesselation shader so i'd have to rewrite my AssetsManager _just_ for
// that, it just make more sense to hard code the shaders.

static const char VERTEX_SRC[] = R"(
#version 410 core
layout(location = 0) in vec4 inPosition;

void main() {
	// Pass position along to the next stage. The actual work is done in the
	// Tessellation Evaluation shader.
	gl_Position = inPosition;
}

)";
static const char TESS_EVAL_SRC[] = R"(
#version 410 core
precision highp float;
layout(quads, equal_spacing) in;
// Per-quad input variables from the vertex shader.

out vec3 frag_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int n_control_points;
uniform int resolution;

const int Max_Control_Points = 32;

struct Control_Point {
	vec3 pos;
};
uniform Control_Point control_points[Max_Control_Points];

int factorial(int x){
	int f = 1;
	for (int i = 1; i <= x; ++i){
		f = f * i;
	}
	return f;
}

float binomial(int n, int i){
	return factorial(n) / float(factorial(i) * factorial(n - i));
}

float bernstein(int n, int i, float x){
	return binomial(n, i) * pow(x, float(i)) * pow(1 - x, float(n - i));
}

vec3 get_point(float s, float t) {
	vec3 result = vec3(0);

	for (int i = 0; i < n_control_points; ++i) {
		for (int j = 0; j < n_control_points; ++j) {

			vec3 next = control_points[j + i * n_control_points].pos;
			next *= bernstein(n_control_points - 1, j, s) * bernstein(n_control_points - 1, i, t);

			result += next;
		}
	}

	return result;
}

out vec3 frag_normal;

void main() {
	vec3 p = get_point(gl_TessCoord.x, gl_TessCoord.y);
	vec3 p_x = get_point(gl_TessCoord.x + 0.01, gl_TessCoord.y);
	vec3 p_y = get_point(gl_TessCoord.x, gl_TessCoord.y + 0.01);

	gl_Position = vec4(get_point(
		gl_TessCoord.x,
		gl_TessCoord.y
	), 1);

	gl_Position.xyz *= (1.0 + 2.0 / resolution);
	gl_Position.xy -= (1.0 / resolution);

	gl_Position = gl_Position * model * view * projection;

	frag_pos = gl_Position.xyz;
	frag_normal = normalize(cross(p_x - p, p_y - p));
}
)";

static const char FRAG_SRC[] = R"(
#version 410 core
uniform vec3 surface_color;

in vec3 frag_pos;
in vec3 frag_normal;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gMRA;

void main() {
	gNormal = frag_normal;
	gAlbedoSpec = vec4(surface_color, 0.0);
	gPosition = frag_pos;
	gMRA = vec4(vec3(1), -1);
}
)";

GLuint create_shader(GLenum type, const char* src) {
	GLuint shader = glCreateShader(type);

	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	GLint status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		GLchar log[512] = { 0 };
		glGetShaderInfoLog(shader, 512, nullptr, log);

		printf("Error tesselation shader: %d, %s\n", (int)type, log);

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}
static GLuint create_shader_program(const std::vector<GLuint>& shaders) {
	GLuint program = glCreateProgram();

	for (GLuint shader : shaders) {
		glAttachShader(program, shader);
	}

	glLinkProgram(program);

	GLint status = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (status == GL_FALSE) {
		GLchar log[512] = { 0 };
		glGetProgramInfoLog(program, 512, nullptr, log);

		printf("Error tesselation shader progam: %s\n", log);

		glDeleteProgram(program);
		return 0;
	}

	return program;
}

Surface::Surface(size_t n_point) noexcept : n_control_points(n_point) {
	assert(n_point >= 2);

	for (size_t j = 0; j < n_point; ++j) {
		for (size_t i = 0; i < n_point; ++i) {
			float s = i / (n_point - 1.f);
			float t = j / (n_point - 1.f);

			control_points.push_back(make_child<Model>());
			control_points.back()->set_global_position({ s, t, 0.f});
			control_points.back()->set_object_copy(Object_File::cube({ 0.05f, 0.05f, 0.05f }));
			control_points.back()->set_shader(AM->get_shader("Deferred_Simple"));
		}
	}

	std::vector<Vector2f> quadData{ Vector2f{0, 0} };

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2f) * quadData.size(), quadData.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), 0);

	glEnableVertexAttribArray(0); // Per-quad position.

	std::vector<GLuint> shaders = {
		create_shader(GL_VERTEX_SHADER, VERTEX_SRC),
		create_shader(GL_TESS_EVALUATION_SHADER, TESS_EVAL_SRC),
		create_shader(GL_FRAGMENT_SHADER, FRAG_SRC),
	};

	shader = create_shader_program(shaders);
	color_loc = glGetUniformLocation(shader, "surface_color");
}

Surface::~Surface() noexcept {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(shader);
}

void Surface::update(float) noexcept {
	ImGui::PushID(this);
	defer{ ImGui::PopID(); };

	ImGui::Begin("Surface");
	defer{ ImGui::End(); };

	int r = resolution;
	ImGui::InputInt("Resolution", &r);
	resolution = std::clamp(r, 1, 100);

	ImGui::ColorEdit3("Surface Color", &surface_color.x);
	ImGui::ColorEdit3("Control Color", &control_point_color.x);

	for (auto& x : control_points) {
		x->set_plain_color({UNROLL_3(control_point_color), 1.f});
		x->set_use_plain_color(true);
	}
}


void Surface::opengl_render() noexcept {
	glUseProgram(shader);

	const GLfloat inner_tess_levels[2] = {
		(float)2 + resolution, // inner horizontal
		(float)2 + resolution  // inner vertical
	};

	const GLfloat outer_tess_levels[4] = {
		(float)2 + resolution, // outer left (vertical)
		(float)2 + resolution, // outer bottom (horizontal)
		(float)2 + resolution, // outer right (vertical)
		(float)2 + resolution  // outer top (horizontal)
	};

	// We can define the tessellation levels using glPatchParameter if we don't
	// have a Tessellation Control Shader stage.
	glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, inner_tess_levels);
	glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, outer_tess_levels);

	Matrix4f model = Matrix4f::translation(get_global_position3());
	Matrix4f view = Window_Info.active_camera->get_view_matrix();
	Matrix4f proj = Window_Info.active_camera->get_projection_matrix();

	glUniform3fv(glGetUniformLocation(shader, "color"), 1, &surface_color.x);
	glUniform1i(glGetUniformLocation(shader, "n_control_points"), n_control_points);
	glUniform1i(glGetUniformLocation(shader, "resolution"), resolution);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, (float*)&model);
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, (float*)&view);
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, (float*)& proj);

	for (size_t i = 0; i < control_points.size(); ++i) {
		std::string str = "control_points[" + std::to_string(i) + "].pos";
		auto p = control_points[i]->get_global_position3();
		glUniform3fv(
			glGetUniformLocation(shader, str.c_str()), 1, &p.x
		);
	}
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glPatchParameteri(GL_PATCH_VERTICES, 1);

	// Draw the tessellated quads.
	glUniform3fv(color_loc, 1, &surface_color.x);
	glDrawArrays(GL_PATCHES, 0, 1);

	// Draw the tessellated quad primitives as wireframe.
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUniform3f(color_loc, 1.0f, 1.0f, 1.0f);
	glDrawArrays(GL_PATCHES, 0, 1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
