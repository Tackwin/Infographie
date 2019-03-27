#pragma once
#include "Model.hpp"
#include <unordered_map>

class Light_Point : public Model {
	static std::unordered_map<Light_Point*, size_t> Shader_Index_Map;


public:
	Light_Point() noexcept;
	~Light_Point() noexcept;

	virtual void update(float) noexcept override;

	virtual void opengl_render() noexcept override;
	virtual void last_opengl_render() noexcept override;

	void set_light_color(Vector3f light_color) noexcept;
	Vector3f get_light_color() const noexcept;

	float get_strength() const noexcept;
	void set_strength(float x) noexcept;

private:
	float strength{ 1 };
	Vector3f light_color;

};

