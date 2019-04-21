#pragma once
#include <vector>

class Widget3;
class Model;
class Surface : public Widget3 {
public:
	Surface(size_t n_point) noexcept;
	virtual ~Surface() noexcept;

	virtual void update(float dt) noexcept override;

	virtual void opengl_render() noexcept override;


private:

	size_t n_control_points;
	std::vector<Model*> control_points;

	Vector3f surface_color;
	Vector3f control_point_color;

	size_t resolution{ 5 };

	size_t vao;
	size_t vbo;

	size_t shader;
	size_t color_loc;
};