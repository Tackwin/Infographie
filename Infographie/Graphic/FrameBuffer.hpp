#pragma once
#include "Common.hpp"
#include "Math/Vector.hpp"

// >Note: Tackwin
// Every time i do a class wrapper around an OpenGl concept i try to have const mean doesn't
// change the OpenGL state. Meaning technically most of the method could be const as they don't
// mutate any members but they do alters the opengl state.
struct G_Buffer {
	G_Buffer(G_Buffer&) = delete;
	G_Buffer& operator=(G_Buffer&) = delete;
	G_Buffer(G_Buffer&&) = default;
	G_Buffer& operator=(G_Buffer&&) = default;

	G_Buffer(Vector2u size) noexcept;
	~G_Buffer() noexcept;

	void set_active() noexcept;
	void set_active_texture() noexcept;
	void set_disable_texture() noexcept;

	void render_quad() noexcept;
	void copy_depth() noexcept;
private:
	Vector2u size;

	uint32_t g_buffer;
	uint32_t pos_buffer;
	uint32_t normal_buffer;
	uint32_t albedo_buffer;
	uint32_t depth_rbo;
	uint32_t quad_VAO;
	uint32_t quad_VBO;
};
