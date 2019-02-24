#pragma once

#include "Widget.hpp"

class Camera : public Widget3 {
public:

	void update(float dt) noexcept override;
	void set_gl_view() noexcept;

	void look_at(Vector3f p, float dist) noexcept;
private:

	Vector3f look_dir{ 0, 0, 1 };
	float speed = 1.f;
};

