#include "LightPoint.hpp"

#include "Managers/AssetsManager.hpp"
decltype(Light_Point::Shader_Index_Map) Light_Point::Shader_Index_Map;

Light_Point::Light_Point() noexcept : Model(true) {
	set_selectable(true);

	set_object_copy(Object_File::cube({ 1, 1, 1 }));
	set_shader(AM->get_shader("Light_Box"));

	// we get the first free index
	size_t i{ 0 };
	while (true) {
top_while: // no project would be truly complete without the use of a goto;
		for (auto& [_, v] : Shader_Index_Map) {
			if (v == i) {
				++i;
				goto top_while;
			}
		}
		break;
	}
	Shader_Index_Map.insert({ this, i});
}

Light_Point::~Light_Point() noexcept {
	Shader_Index_Map.erase(this);
}

void Light_Point::update(float) noexcept {
	auto idx = std::to_string(Shader_Index_Map.at(this));
	auto str = "lights[" + idx + "].";

	AM->get_shader("Deferred_Light").setUniform(
		str + "Position", sf::Vector3f{ UNROLL_3(get_global_position3()) }
	);
	AM->get_shader("Deferred_Light").setUniform(
		str + "Color", sf::Vector3f{ UNROLL_3(light_color) }
	);
	// update attenuation parameters and calculate radius
	// note that we don't send this to the shader, we assume it is always 1.0 (in our case)
	const float constant = 1.0f;
	const float linear = 0.7f;
	const float quadratic = 1.8f;
	AM->get_shader("Deferred_Light").setUniform(str + "Linear", linear);
	AM->get_shader("Deferred_Light").setUniform(str + "Quadratic", quadratic);
}

// We make this is a noop since we render the body of a light in the last opengl render pass.
void Light_Point::opengl_render() noexcept {}

// _Now_ we render as if we were in the opengl render pass
void Light_Point::last_opengl_render() noexcept {
	shader->setUniform("light_color", sf::Vector3f{ UNROLL_3(light_color) });
	Model::last_opengl_render();
	Model::opengl_render();
}

void Light_Point::set_light_color(Vector3f l) noexcept {
	light_color = l;
}
Vector3f Light_Point::get_light_color() const noexcept {
	return light_color;
}

