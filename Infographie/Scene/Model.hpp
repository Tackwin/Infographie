#pragma once
#include "Widget.hpp"

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include "Math/Vector.hpp"
#include "Files/FileFormat.hpp"


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
	void set_alpha_texture(const sf::Texture& texture) noexcept;
	void set_shader(sf::Shader& shader) noexcept;
	void set_select_shader(sf::Shader& shader) noexcept;

	const sf::Texture* get_texture() const noexcept;

	float get_picking_sphere_radius() const noexcept;

	size_t get_n() const noexcept;
	virtual void set_size(Vector3f size) noexcept override;

	virtual std::optional<Vector3f>
	is_selected(Vector3f ray_origin, Vector3f ray) const noexcept override;

	virtual void set_focus(bool v) noexcept override;

	void set_scaling(Vector3f s) noexcept;
	Vector3f get_scaling() const noexcept;

	void set_selectable(bool v) noexcept;

protected:
	void toggle_picker() noexcept;
	void push_picker() noexcept;
	void pop_picker() noexcept;

	float picking_sphere_radius{ 0.f };

	size_t n;

	sf::Shader* shader{ nullptr };
	sf::Shader* select_shader{ nullptr };
	const sf::Texture* texture{ nullptr };
	const sf::Texture* alpha_texture{ nullptr };

	// For complexe object we take a reference from the Assets Manager
	const Object_File* object_file{ nullptr };

	// For simple one that we generate ourself (like the bounding box cube)
	// we use a local copy
	Object_File object_file_copy;

	bool selectable{ true };

	bool render_checkbox{ false };
	bool without_bounding_box{ false };

	std::optional<GLuint> vertex_array_id;
	std::optional<GLuint> vertex_buffer_id;
	std::optional<GLuint> uv_buffer_id;
	std::optional<GLuint> normal_buffer_id;

	Model* boundingbox_child{ nullptr };

	struct Picker : public Widget3 {
		std::unique_ptr<Model> xy_plan;
		std::unique_ptr<Model> yz_plan;
		std::unique_ptr<Model> zx_plan;

		enum class Plan_List {
			None = 0,
			XY,
			YZ,
			ZX,
			Count
		} selected_plan = Plan_List::None;
		Vector3f initial_pos;

		Picker() noexcept;

		virtual void update(float) noexcept override;

		virtual void last_opengl_render() noexcept override;
	};
	Picker* picker{ nullptr };

	Vector3f scaling{ 1, 1, 1 };
};
