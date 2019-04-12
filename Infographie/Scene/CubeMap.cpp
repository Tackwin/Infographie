#include <GL/glew.h>

#include "CubeMap.hpp"

#include "Managers/AssetsManager.hpp"

#include "Files/stb_image.h"

Cube_Map::Cube_Map() noexcept : Widget3() {
	glGenTextures(1, &texture_id);

	cube_model.set_object_copy(Object_File::cube({ 5, 5, 5 }));
	cube_model.set_shader(AM->get_shader("Skybox"));
}

void Cube_Map::opengl_render() noexcept {
	glDepthMask(GL_FALSE);
	defer{
		glDepthMask(GL_TRUE);
	};
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

	// we short circuit the normal render cycle because a skybox doesn't have a position
	// (or i guess, the camera position), doesn't rotate or in a general way transform
	// but Model still has cool logic internally (it keep track of the VAO ...) so we still use
	// the Model class. In the same vein we don't let it bind his texture because we use
	// a different type of texture (namely GL_TEXTURE_CUBE_MAP)
	cube_model.opengl_render();
}

[[nodiscard]]
bool Cube_Map::load_texture(std::filesystem::path path) noexcept {
	if (!std::filesystem::is_regular_file(path)) return false;

	stbi_set_flip_vertically_on_load(true);
	Vector2i size;
	int n_comp;
	auto* data = stbi_loadf(path.generic_string().c_str(), &size.x, &size.y, &n_comp, 0);
	if (!data) return false;
	defer{ stbi_image_free(data); };

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_FLOAT, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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


