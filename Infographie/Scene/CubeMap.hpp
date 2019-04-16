#pragma once
#include "Common.hpp"

#include "Widget.hpp"

#include "Model.hpp"

namespace sf {
	class Image;
}
struct Cube_Map : Widget3 {

	Cube_Map() noexcept;

	virtual void last_opengl_render() noexcept override;

	void set_textures(const sf::Image data[6]) noexcept;

	[[nodiscard]]
	bool load_texture(std::filesystem::path path) noexcept;

	void set_name(std::string str) noexcept;
	const std::string& get_name() const noexcept;

	[[nodiscard]] size_t get_irradiance_id() const noexcept;
	[[nodiscard]] size_t get_prefilter_id() const noexcept;
	[[nodiscard]] size_t get_brdf_lut_id() const noexcept;
private:
	Model cube_model;

	size_t texture_id;
	size_t irradiance_id;
	size_t prefilter_id;
	static std::optional<size_t> brdf_lut_id;
	std::string name;
};
