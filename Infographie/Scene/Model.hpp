#pragma once

#include "Widget.hpp"
#include "Math/Vector.hpp"
#include "Files/FileFormat.hpp"

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include <optional>

class Model : public Widget3 {
public:
	static size_t Total_N;

	Model() noexcept;
	~Model() noexcept override;
	// >Tackwin
	// I'm going to use the update function since i can't mix the render cycle of sfml
	// with that of opengl
	// I KNOW shut up, i'll make a separate cycle later...
	// I just want to see thoses sweet triangles
	void update(float dt) noexcept;

	void set_object(const Object_File& object_file) noexcept;
	void set_texture(const sf::Texture& texture) noexcept;
	void set_shader(sf::Shader& shader) noexcept;

	const sf::Texture* get_texture() const noexcept;

	size_t get_n() const noexcept;
private:
	size_t n;

	sf::Shader* shader;
	const sf::Texture* texture;
	const Object_File* object_file;

	std::optional<GLuint> vertex_array_id;
	std::optional<GLuint> vertex_buffer_id;
	std::optional<GLuint> uv_buffer_id;
	std::optional<GLuint> normal_buffer_id;

};
