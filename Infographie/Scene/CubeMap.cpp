#include <GL/glew.h>

#include "CubeMap.hpp"

#include "Managers/AssetsManager.hpp"

#include "Files/stb_image.h"

#include "Math/Matrix.hpp"

#include "OS/PathDefinition.hpp"

Cube_Map::Cube_Map() noexcept : Widget3() {
	glGenTextures(1, &texture_id);

	cube_model.set_object_copy(Object_File::cube({ 5, 5, 5 }));
	cube_model.set_shader(AM->get_shader("Skybox"));
}

void Cube_Map::last_opengl_render() noexcept {
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

	sf::Shader::bind(&AM->get_shader("Skybox"));
	AM->get_shader("Skybox").setUniform("skybox", 0);

	// we short circuit the normal render cycle because a skybox doesn't have a position
	// (or i guess, the camera position), doesn't rotate or in a general way transform
	// but Model still has cool logic internally (it keep track of the VAO ...) so we still use
	// the Model class. In the same vein we don't let it bind his texture because we use
	// a different type of texture (namely GL_TEXTURE_CUBE_MAP)
	cube_model.opengl_render();
}

[[nodiscard]]
bool Cube_Map::load_texture(std::filesystem::path path) noexcept {
	constexpr Vector2u Cubemap_Texture_Size{ 2048, 2048 };

	if (!std::filesystem::is_regular_file(path)) return false;

	std::filesystem::current_path(Base_Working_Directory);

	if (!AM->have_shader("Irradiance")) {
		AM->load_shader(
			"Irradiance", "res/shaders/cubemap.vertex", "res/shaders/irradiance.fragment"
		);
	}
	if (!AM->have_shader("Equi_To_Cube")) {
		AM->load_shader(
			"Equi_To_Cube", "res/shaders/cubemap.vertex", "res/shaders/equi_to_cube.fragment"
		);
	}

	auto& equi_to_cube_shader = AM->get_shader("Equi_To_Cube");
	auto& irradiance_shader = AM->get_shader("Irradiance");

	// pbr: setup framebuffer
// ----------------------
	unsigned int captureFBO;
	unsigned int captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	defer{
		glDeleteFramebuffers(1, &captureFBO);
		glDeleteRenderbuffers(1, &captureRBO);
	};

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(
		GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Cubemap_Texture_Size.x, Cubemap_Texture_Size.y
	);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	// pbr: load the HDR environment map
	// ---------------------------------
	int width, height, nrComponents;
	float* data = stbi_loadf(path.generic_string().c_str(), &width, &height, &nrComponents, 0);
	defer{ stbi_image_free(data); };

	unsigned int hdrTexture;

	if (!data) return false;

	glGenTextures(1, &hdrTexture);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	// note how we specify the texture's data value to be float
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	defer{ glDeleteTextures(1, &hdrTexture); };

	// pbr: setup cubemap to render to and attach to framebuffer
	// ---------------------------------------------------------
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			Cubemap_Texture_Size.x,
			Cubemap_Texture_Size.y,
			0,
			GL_RGB,
			GL_FLOAT,
			nullptr
		);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// pbr: set up projection and view matrices for capturing data onto the 6 face directions
	// --------------------------------------------------------------------------------------
	auto captureProjection = Matrix4f::perspective(PIf / 2, 1, 10, 0.1f);
	Matrix4f captureViews[] = {
		Matrix4f::look_at({0.0f, 0.0f, 0.0f}, { 1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f,  0.0f}),
		Matrix4f::look_at({0.0f, 0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f,  0.0f}),
		Matrix4f::look_at({0.0f, 0.0f, 0.0f}, { 0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f,  1.0f}),
		Matrix4f::look_at({0.0f, 0.0f, 0.0f}, { 0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f, -1.0f}),
		Matrix4f::look_at({0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f,  0.0f}),
		Matrix4f::look_at({0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f,  0.0f})
	};

	// pbr: convert HDR equirectangular environment map to cubemap equivalent
	// ----------------------------------------------------------------------
	sf::Shader::bind(&equi_to_cube_shader);
	equi_to_cube_shader.setUniform("equirectangularMap", 0);
	glUniformMatrix4fv(
		glGetUniformLocation(equi_to_cube_shader.getNativeHandle(), "projection"),
		1,
		GL_FALSE,
		(float*)& captureProjection
	);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	//don't forget to configure the viewport to the capture dimensions.
	glViewport(0, 0, Cubemap_Texture_Size.x, Cubemap_Texture_Size.y);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	
	auto cube = Object_File::cube({ 2, 2, 2 });

	size_t cubeVAO{0};
	size_t cubeVBO{0};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	// fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		cube.vertices.size() * sizeof(Vector3f),
		cube.vertices.data(),
		GL_STATIC_DRAW
	);
	// link vertex attributes
	glBindVertexArray(cubeVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	for (unsigned int i = 0; i < 6; ++i) {
		glUniformMatrix4fv(
			glGetUniformLocation(equi_to_cube_shader.getNativeHandle(), "view"),
			1,
			GL_FALSE,
			(float*)&captureViews[i]
		);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, texture_id, 0
		);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
	// --------------------------------------------------------------------------------
	glGenTextures(1, &irradiance_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_id);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr
		);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
	// -----------------------------------------------------------------------------
	sf::Shader::bind(&irradiance_shader);
	irradiance_shader.setUniform("environmentMap", 0);
	glUniformMatrix4fv(
		glGetUniformLocation(irradiance_shader.getNativeHandle(), "projection"),
		1,
		GL_FALSE,
		(float*)&captureProjection
	);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i) {
		glUniformMatrix4fv(
			glGetUniformLocation(irradiance_shader.getNativeHandle(), "view"),
			1,
			GL_FALSE,
			(float*)& captureViews[i]
		);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			irradiance_id,
			0
		);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void Cube_Map::set_textures(const sf::Image data[6]) noexcept {
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

	for (size_t i = 0; i < 6; ++i) {
		auto& x = data[i];
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGBA,
			x.getSize().x,
			x.getSize().y,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			x.getPixelsPtr()
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

const std::string& Cube_Map::get_name() const noexcept {
	return name;
}

void Cube_Map::set_name(std::string str) noexcept {
	name = std::move(str);
}


