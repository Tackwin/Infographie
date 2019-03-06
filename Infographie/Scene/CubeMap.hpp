#pragma once
#include "Common.hpp"

#include "Widget.hpp"

#include "Model.hpp"

namespace sf {
	class Image;
}
struct Cube_Map : Widget3 {

	Cube_Map() noexcept;

	virtual void opengl_render() noexcept override;

	void set_textures(const sf::Image data[6]) noexcept;

	void set_name(std::string str) noexcept;
	const std::string& get_name() const noexcept;
private:
	Model cube_model;

	size_t texture_id;
	std::string name;
};
