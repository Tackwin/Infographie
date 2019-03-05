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

	Model(bool without_bounding_box = false) noexcept;
	~Model() noexcept override;

	virtual void opengl_render() noexcept override;

	bool does_render_checkbox() noexcept;
	void set_render_checkbox(bool v) noexcept;

	void set_object(const Object_File& object_file) noexcept;
	void set_object_copy(const Object_File& object_file) noexcept;
	void set_texture(const sf::Texture& texture) noexcept;
	void set_shader(sf::Shader& shader) noexcept;
	void set_select_shader(sf::Shader& shader) noexcept;

	const sf::Texture* get_texture() const noexcept;

	float get_picking_sphere_radius() const noexcept;

	size_t get_n() const noexcept;
	virtual void set_size(Vector3f size) noexcept override;

	virtual std::optional<Vector3f>
	is_selected(Vector3f ray_origin, Vector3f ray) const noexcept override;

	void set_scaling(Vector3f s) noexcept;
	Vector3f get_scaling() const noexcept;

private:
	float picking_sphere_radius{ 0.f };

	size_t n;

	sf::Shader* shader;
	sf::Shader* select_shader;
	const sf::Texture* texture;

	// For complexe object we take a reference from the Assets Manager
	const Object_File* object_file;

	// For simple one that we generate ourself (like the bounding box cube)
	// we use a local copy
	Object_File object_file_copy;

	bool render_checkbox{ false };
	bool without_bounding_box{ false };

	std::optional<GLuint> vertex_array_id;
	std::optional<GLuint> vertex_buffer_id;
	std::optional<GLuint> uv_buffer_id;
	std::optional<GLuint> normal_buffer_id;

	Model* boundingbox_child{ nullptr };

	Vector3f scaling{ 1, 1, 1 };
};
