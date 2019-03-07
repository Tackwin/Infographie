#include "AssetsManager.hpp"
#include "Common.hpp"

#include <filesystem>
#include <cassert>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
void stubSetConsoleTextAttribute(HANDLE h, WORD w) {
	SetConsoleTextAttribute(h, w);
}
#else
void stubSetConsoleTextAttibute() {
	static bool flag = false;
	if (!flag) {
		printf("Color console not yet supported!\n");
		flag = true;
	}
};
#endif

bool Assets_Manager::have_texture(const std::string& key) noexcept {
	return textures.find(key) != std::end(textures);
}

sf::Texture& Assets_Manager::create_texture(const std::string& key) noexcept {
	assert(textures.find(key) == std::end(textures));
	return textures[key];
}

bool Assets_Manager::load_texture(
	const std::string& key, const std::filesystem::path& path
) noexcept {
	if (textures.find(key) != std::end(textures))
		return true;

	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE), 
		FOREGROUND_RED | FOREGROUND_BLUE
	);
	std::printf("Loading: ");
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE), 
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	auto &ref = textures[key];
	ref.setSmooth(true);
	std::printf("%s: %ls ", key.c_str(), path.c_str());

	const bool loaded = ref.loadFromFile(path.generic_string());
	if(!loaded) {
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE), 
			FOREGROUND_RED
		);
		printf(" Couldn't load file /!\\\n");
	}
	else {
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE), 
			FOREGROUND_GREEN
		);
		printf(" Succes !\n");
	}
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE), 
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	return loaded;
}

/*const*/ sf::Texture& Assets_Manager::get_texture(const std::string &key) noexcept {
	auto it = textures.find(key);
	assert(it != std::end(textures) && "Texture don't exist");
	return it->second;
}

bool Assets_Manager::have_image(const std::string& key) noexcept {
	return images.find(key) != std::end(images);
}
bool Assets_Manager::load_image(const std::string &key, const std::string &path) noexcept {
	if (images.find(key) != std::end(images))
		return true;

	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE), 
		FOREGROUND_RED | FOREGROUND_BLUE
	);
	std::printf("Loading: ");
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE), 
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	std::printf("%s: %s ", key.c_str(), path.c_str());
	auto& ref = images[key];

	const bool loaded = ref.loadFromFile(path);
	if(!loaded) {
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE), 
			FOREGROUND_RED
		);
		printf(" Couldn't load file /!\\\n");
	} else {
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE), 
			FOREGROUND_GREEN
		);
		printf(" Succes !\n");
	}
	SetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE), 
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	return loaded;
}

const sf::Image& Assets_Manager::get_image(const std::string &key) noexcept {
	auto it = images.find(key);
	assert(it != std::end(images) && "Image don't exist");
	return it->second;
}

bool Assets_Manager::have_font(const std::string& key) noexcept {
	return fonts.find(key) != std::end(fonts);
}
bool Assets_Manager::load_font(const std::string &key, const std::string &path) noexcept {
	if (fonts.find(key) != std::end(fonts))
		return true;
	
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE), 
		FOREGROUND_RED | FOREGROUND_BLUE
	);
	std::printf("Loading: ");
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE), 
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	std::printf("%s: %s ", key.c_str(), path.c_str());
	auto& ref = fonts[key];

	const bool loaded = ref.loadFromFile(path);
	if(!loaded) {
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE), 
			FOREGROUND_RED
		);
		printf(" Couldn't load file /!\\\n");
	} else {
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE), 
			FOREGROUND_GREEN
		);
		printf(" Succes !\n");
	}
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE), 
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	return loaded;
}

const sf::Font& Assets_Manager::get_font(const std::string &key) noexcept {
	auto it = fonts.find(key);
	assert(it != std::end(fonts) && "Font don't exist");
	return it->second;
}


bool Assets_Manager::have_object_file(const std::string& key) noexcept {
	return objects.find(key) != std::end(objects);
}
bool Assets_Manager::load_object_file(
	const std::string& key, const std::filesystem::path& path
) noexcept {
	if (objects.find(key) != std::end(objects))
		return true;

	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_BLUE
	);
	std::printf("Loading: ");
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	std::printf("%s: %s ", key.c_str(), path.generic_string().c_str());
	auto& ref = objects[key];

	auto loaded = Object_File::load_file(path);
	if (!loaded) {
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_RED
		);
		printf(" Couldn't load file /!\\\n");
	}
	else {
		ref = *loaded;
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_GREEN
		);
		printf(" Succes !\n");
	}
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	return loaded.has_value();
}
const Object_File& Assets_Manager::get_object_file(const std::string& key) noexcept {
	auto it = objects.find(key);
	assert(it != std::end(objects) && "Object File don't exist (wasn't loaded)");
	return it->second;
}

bool Assets_Manager::have_shader(const std::string& key) noexcept {
	return shaders.find(key) != std::end(shaders);
}
bool Assets_Manager::load_shader(
	const std::string& key,
	const std::filesystem::path& vertex,
	const std::filesystem::path& fragment
) noexcept {
	if (shaders.find(key) != std::end(shaders))
		return true;

	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_BLUE
	);
	std::printf("Loading: ");
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	std::printf(
		"%s: %s %s",
		key.c_str(),
		vertex.generic_string().c_str(),
		fragment.generic_string().c_str()
	);
	auto & ref = shaders[key];

	auto loaded = ref.loadFromFile(vertex.generic_string(), fragment.generic_string());
	if (!loaded) {
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_RED
		);
		printf(" Couldn't load file /!\\\n");
	}
	else {
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_GREEN
		);
		printf(" Succes !\n");
	}
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	return loaded;
}
bool Assets_Manager::load_shader(
	const std::string& key,
	const std::filesystem::path& vertex,
	const std::filesystem::path& fragment,
	const std::filesystem::path& geometry
) noexcept {
	if (shaders.find(key) != std::end(shaders))
		return true;

	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_BLUE
	);
	std::printf("Loading: ");
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	std::printf(
		"%s: %s %s",
		key.c_str(),
		vertex.generic_string().c_str(),
		fragment.generic_string().c_str()
	);
	auto & ref = shaders[key];

	auto loaded = ref.loadFromFile(
		vertex.generic_string(), geometry.generic_string(), fragment.generic_string()
	);
	if (!loaded) {
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_RED
		);
		printf(" Couldn't load file /!\\\n");
	}
	else {
		stubSetConsoleTextAttribute(
			GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_GREEN
		);
		printf(" Succes !\n");
	}
	stubSetConsoleTextAttribute(
		GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);
	return loaded;
}
sf::Shader& Assets_Manager::get_shader(const std::string& key) noexcept {
	auto it = shaders.find(key);
	assert(it != std::end(shaders) && "Object File don't exist (wasn't loaded)");
	return it->second;
}

