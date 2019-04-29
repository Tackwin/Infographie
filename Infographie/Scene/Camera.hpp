#pragma once

#include "Common.hpp"
#include "Widget.hpp"

#include "Math/Matrix.hpp"

#include "Graphic/FrameBuffer.hpp"

#include "Scene/Cubemap.hpp"

#include "UI/RayTracing.hpp"

class Camera : public Widget3 {
public:
	Camera() noexcept;

	void render_from(Widget3* root) noexcept;
	Widget3* get_render_root() const noexcept;

	virtual void render(Texture_Buffer& target) noexcept override;

	void update(float dt) noexcept override;

	void set_speed(float speed) noexcept;

	void set_viewport(Rectangle2f rec) noexcept;
	void set_gl_view() noexcept;

	void look_at(Vector3f target, Vector3f up) noexcept;
	void set_perspective(float fov, float ratio, float f, float n) noexcept;
	void set_orthographic(float scale, float ratio, float f, float n) noexcept;

	// Lock view to those widgets, they must be a child of render_root.
	// we simply take the (max + min) / 2 point to lock.
	void lock(std::vector<Uuid_t> id) noexcept;

	const Matrix4f& get_view_matrix() noexcept;
	const Matrix4f& get_projection_matrix() noexcept;

	Vector2f get_mouse_viewport() const noexcept;

	bool is_input_active() const noexcept;
	void set_input_active(bool v) noexcept;

	float get_exposure() const noexcept;
	void set_exposure(float x) noexcept;
	float get_gamma() const noexcept;
	void set_gamma(float x) noexcept;

	// All the parameters that get eveluated each frame(ie. the one that don't need cached value
	// that would be set in a setter) i just make them public it's easier and express intent.

	float pitch{ 0 };
	float yaw{ PIf };

	float cam_far{ 500 };
	float cam_near{ .01f };
	
	float speed{ 15 };

	float exposure{ 1.f };
	float gamma{ 1.f };

	Vector3f look_dir{ 0, 0, 1 };
	Vector3f right{ 1, 0, 0 };
	Vector3f up{ 0, 1, 0 };

	Ray_Tracing_Settings ray_tracing_settings;
	
private:
	int Show_Debug{ 0 };
	int Show_Debug_SSAO{ 0 };
	bool Active_Debug{ true };

	void select_ray_cast() noexcept;
	void compute_view() noexcept;

	Cube_Map* find_active_cubemap() const noexcept;

	Widget3* render_root{ nullptr };

	Matrix4f view;
	Matrix4f projection;
	Rectangle2f viewport;

	bool input_active{ false };

	std::vector<Uuid_t> ids_to_lock;

	G_Buffer g_buffer;
	HDR_Buffer hdr_buffer;
	SSAO_Buffer ssao_buffer;
};

