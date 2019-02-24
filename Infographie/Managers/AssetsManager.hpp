#pragma once
#include <unordered_map>
#include <memory>
#include <filesystem>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Files/FileFormat.hpp"

class Assets_Manager {
public:
	bool have_texture(const std::string& key) noexcept;
	bool load_texture(const std::string& key, const std::filesystem::path& path) noexcept;
	// TODO<
	// is const really nedded here ?
	// i don't want somebody to change a texture for everyone else
	// but then i can't set the smoothness of a texture for instance
	/*const*/ sf::Texture& get_texture(const std::string& key) noexcept; 
	bool have_image(const std::string& key) noexcept;
	bool load_image(const std::string& key, const std::string& path) noexcept;
	const sf::Image& get_image(const std::string& key) noexcept;

	bool have_font(const std::string& key) noexcept;
	bool load_font(const std::string& key, const std::string& path) noexcept;
	const sf::Font& get_font(const std::string& key) noexcept;

	bool have_object_file(const std::string& key) noexcept;
	bool load_object_file(const std::string& key, const std::filesystem::path& path) noexcept;
	const Object_File& get_object_file(const std::string& key) noexcept;

	bool have_shader(const std::string& key) noexcept;
	bool load_shader(
		const std::string& key,
		const std::filesystem::path& vertex,
		const std::filesystem::path& fragment
	) noexcept;
	sf::Shader& get_shader(const std::string& key) noexcept;

private:
	std::unordered_map<std::string, sf::Texture> textures;
	std::unordered_map<std::string, Object_File> objects;
	std::unordered_map<std::string, sf::Shader> shaders;
	std::unordered_map<std::string, sf::Image> images;
	std::unordered_map<std::string, sf::Font> fonts;
};
