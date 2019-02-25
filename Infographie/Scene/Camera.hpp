#pragma once

#include "Widget.hpp"

#include "Math/Matrix.hpp"

class Camera : public Widget3 {
public:
	Camera() noexcept;

	void update(float dt) noexcept override;

	void set_viewport(Rectangle2u rec) noexcept;
	void set_gl_view() noexcept;

	void look_at(Vector3f target) noexcept;
	void set_perspective(float fov, float ratio, float f, float n) noexcept;

	const Matrix4f& get_view_matrix() noexcept;
	const Matrix4f& get_projection_matrix() noexcept;
private:

	Matrix4f view;
	Matrix4f projection;
	Rectangle2u viewport;
	Vector3f look_dir{ 0, 0, 1 };
	float speed = 5.f;
};

