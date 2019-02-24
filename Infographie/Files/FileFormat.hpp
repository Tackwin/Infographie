#pragma once
#include <vector>
#include <optional>
#include <filesystem>

#include "Math/Vector.hpp"

struct Object_File {
	std::vector<Vector2f> uvs;
	std::vector<Vector3f> normals;
	std::vector<Vector3f> vertices;

	Vector3f min;
	Vector3f max;

	static std::optional<Object_File> load_file(const std::filesystem::path& path) noexcept;
};
