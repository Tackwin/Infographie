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

	uint32_t g_buffer{ 0 };
	uint32_t pos_buffer{ 0 };
	uint32_t normal_buffer{ 0 };
	uint32_t albedo_buffer{ 0 };
	uint32_t depth_rbo{ 0 };
	uint32_t quad_VAO{ 0 };
	uint32_t quad_VBO{ 0 };
};

struct HDR_Buffer {
	HDR_Buffer(HDR_Buffer&) = delete;
	HDR_Buffer& operator=(HDR_Buffer&) = delete;
	HDR_Buffer(HDR_Buffer&&) = default;
	HDR_Buffer& operator=(HDR_Buffer&&) = delete;

	HDR_Buffer(Vector2u size) noexcept;
	~HDR_Buffer() noexcept;

	Vector2u get_size() const noexcept;

	void set_active() noexcept;
	void set_active_texture() noexcept;
	void set_disable_texture() noexcept;

	void render_quad() noexcept;
private:
	Vector2u size;

	uint32_t hdr_buffer{ 0 };
	uint32_t rbo_buffer{ 0 };
	uint32_t color_buffer{ 0 };
	uint32_t quad_VAO{ 0 };
	uint32_t quad_VBO{ 0 };
};
