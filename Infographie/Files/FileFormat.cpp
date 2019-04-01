#include "FileFormat.hpp"

#include <array>

#include "OS/FileIO.hpp"
#include "Math/algorithms.hpp"

std::optional<Object_File> Object_File::load_file(const std::filesystem::path& path) noexcept {
	if (!std::filesystem::is_regular_file(path)) return std::nullopt;
	constexpr auto Line_Comment_Char = '#';
	constexpr auto Mtllib = std::array<char, 7>{ "mtllib" };
	constexpr auto Usemtl = std::array<char, 7>{ "usemtl" };
	constexpr auto Vertex_Char = 'v';
	constexpr auto Texture_Char = std::array<char, 3>{ "vt" };
	constexpr auto Normal_Char = std::array<char, 3>{ "vn" };
	constexpr auto Face_Char = 'f';

	auto opt_bytes = read_whole_file(path);
	if (!opt_bytes) return std::nullopt;
	auto& bytes = *opt_bytes;

	Object_File obj;
	std::vector<Vector3<Vector3u>> faces;

	for (size_t i = 0; i < bytes.size() - 1;) {
		// skip the whole line we do nothing at the end of each iteration we go _past_ the '\n'
		if (bytes[i] == Line_Comment_Char) {
		}
		else
		// the -1 is for the '\0' at the end of Mtllib
		if (
			bytes.size() > i + (Mtllib.size() - 1) &&
			memcmp(bytes.data() + i, Mtllib.data(), Mtllib.size() - 1) == 0
		) {
		}
		else
		if (
			bytes.size() > i + (Usemtl.size() - 1) &&
			memcmp(bytes.data() + i, Usemtl.data(), Usemtl.size() - 1) == 0
		) {
		}
		else
		if (bytes[i] == Face_Char) {
			i++;
			if (bytes.size() <= i) return std::nullopt;

			Vector3<Vector3u> face;
			auto read = sscanf_s(
				bytes.data() + i,
				"%u/%u/%u %u/%u/%u %u/%u/%u",
				&face.x.x,
				&face.x.y,
				&face.x.z,
				&face.y.x,
				&face.y.y,
				&face.y.z,
				&face.z.x,
				&face.z.y,
				&face.z.z
			);
			if (read == EOF) return std::nullopt;
			if (read != 9) return std::nullopt;

			faces.push_back(face);
		}
		else
		if (
			bytes.size() > i + Texture_Char.size() - 1 &&
			memcmp(bytes.data() + i, Texture_Char.data(), Texture_Char.size() - 1) == 0
		) {
			i += 2;
			if (bytes.size() <= i) return std::nullopt;
			Vector2f vec;

			auto read = sscanf_s(bytes.data() + i, "%f %f", &vec.x, &vec.y);
			if (read == EOF) return std::nullopt;
			if (read != 2) return std::nullopt;

			// >SEE we use sf::Texture for our texture and they are top down.
			vec.y = 1 - vec.y;

			obj.uvs.push_back(vec);
		}
		else
		if (
			bytes.size() > i + Normal_Char.size() - 1 &&
			memcmp(bytes.data() + i, Normal_Char.data(), Normal_Char.size() - 1) == 0
		) {
			i += 2;
			if (bytes.size() <= i) return std::nullopt;
			Vector3f vec;

			auto read = sscanf_s(bytes.data() + i, "%f %f %f", &vec.x, &vec.y, &vec.z);
			if (read == EOF) return std::nullopt;
			if (read != 3) return std::nullopt;

			obj.normals.push_back(vec);
		}
		else
		if (bytes[i] == Vertex_Char) {
			i++;
			if (bytes.size() <= i) return std::nullopt;
			Vector3f vec;

			auto read = sscanf_s(bytes.data() + i, "%f %f %f", &vec.x, &vec.y, &vec.z);
			if (read == EOF) return std::nullopt;
			if (read != 3) return std::nullopt;


			obj.vertices.push_back(vec);
		}

		// otherwise we just ignore it. and move _past_ the next line !!
		while (i < bytes.size() - 1 && bytes[i++] != '\n');
	}

	std::vector<Vector3f> indexed_vertices;
	std::vector<Vector2f> indexed_uvs;
	std::vector<Vector3f> indexed_normals;

	for (auto& f : faces) {
		indexed_vertices.push_back(obj.vertices[f.x.x - 1]);
		indexed_vertices.push_back(obj.vertices[f.y.x - 1]);
		indexed_vertices.push_back(obj.vertices[f.z.x - 1]);

		static bool first = true;

		if (first || obj.min.x > indexed_vertices.back().x) obj.min.x = indexed_vertices.back().x;
		if (first || obj.min.y > indexed_vertices.back().y) obj.min.y = indexed_vertices.back().y;
		if (first || obj.min.z > indexed_vertices.back().z) obj.min.z = indexed_vertices.back().z;

		if (first || obj.max.x < indexed_vertices.back().x) obj.max.x = indexed_vertices.back().x;
		if (first || obj.max.y < indexed_vertices.back().y) obj.max.y = indexed_vertices.back().y;
		if (first || obj.max.z < indexed_vertices.back().z) obj.max.z = indexed_vertices.back().z;

		first = false;

		indexed_uvs.push_back(obj.uvs[f.x.y - 1]);
		indexed_uvs.push_back(obj.uvs[f.y.y - 1]);
		indexed_uvs.push_back(obj.uvs[f.z.y - 1]);

		indexed_normals.push_back(obj.normals[f.x.z - 1]);
		indexed_normals.push_back(obj.normals[f.y.z - 1]);
		indexed_normals.push_back(obj.normals[f.z.z - 1]);


		auto& pos1 = obj.vertices[f.x.x - 1];
		auto& pos2 = obj.vertices[f.y.x - 1];
		auto& pos3 = obj.vertices[f.z.x - 1];

		auto& uv1 = obj.uvs[f.x.y - 1];
		auto& uv2 = obj.uvs[f.y.y - 1];
		auto& uv3 = obj.uvs[f.z.y - 1];

		auto edge1 = pos2 - pos1;
		auto edge2 = pos3 - pos1;
		auto deltaUV1 = uv2 - uv1;
		auto deltaUV2 = uv3 - uv1;

		float dt = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		Vector3f tangent;
		Vector3f bitangent;

		tangent.x = dt * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = dt * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = dt * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent = tangent.normalize();

		bitangent.x = dt * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = dt * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = dt * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent = bitangent.normalize();

		for (size_t i = 0; i < 3; ++i) {
			obj.tangents.push_back(tangent);
			obj.bitangents.push_back(bitangent);
		}
	}

	obj.normals.swap(indexed_normals);
	obj.uvs.swap(indexed_uvs);
	obj.vertices.swap(indexed_vertices);

	return obj;
}

Object_File Object_File::cube(Vector3f size) noexcept {
	Object_File obj;
	obj.min = size / -2;
	obj.max = size / +2;

	obj.normals.push_back({  0.0f,  0.0f, -1.0f });
	obj.normals.push_back({  0.0f,  0.0f, -1.0f });
	obj.normals.push_back({  0.0f,  0.0f, -1.0f });
	obj.normals.push_back({  0.0f,  0.0f, -1.0f });
	obj.normals.push_back({  0.0f,  0.0f, -1.0f });
	obj.normals.push_back({  0.0f,  0.0f, -1.0f });
	obj.normals.push_back({  0.0f,  0.0f,  1.0f });
	obj.normals.push_back({  0.0f,  0.0f,  1.0f });
	obj.normals.push_back({  0.0f,  0.0f,  1.0f });
	obj.normals.push_back({  0.0f,  0.0f,  1.0f });
	obj.normals.push_back({  0.0f,  0.0f,  1.0f });
	obj.normals.push_back({  0.0f,  0.0f,  1.0f });
	obj.normals.push_back({  -1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  -1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  -1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  -1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  -1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  -1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  1.0f,  0.0f,  0.0f });
	obj.normals.push_back({  0.0f, -1.0f,  0.0f });
	obj.normals.push_back({  0.0f, -1.0f,  0.0f });
	obj.normals.push_back({  0.0f, -1.0f,  0.0f });
	obj.normals.push_back({  0.0f, -1.0f,  0.0f });
	obj.normals.push_back({  0.0f, -1.0f,  0.0f });
	obj.normals.push_back({  0.0f, -1.0f,  0.0f });
	obj.normals.push_back({  0.0f,  1.0f,  0.0f });
	obj.normals.push_back({  0.0f,  1.0f,  0.0f });
	obj.normals.push_back({  0.0f,  1.0f,  0.0f });
	obj.normals.push_back({  0.0f,  1.0f,  0.0f });
	obj.normals.push_back({  0.0f,  1.0f,  0.0f });
	obj.normals.push_back({  0.0f,  1.0f,  0.0f });

	obj.uvs.push_back({ 0.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 1.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 1.0f });
	obj.uvs.push_back({ 0.0f, 0.0f });
	obj.uvs.push_back({ 0.0f, 1.0f });
	obj.uvs.push_back({ 0.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 1.0f });
	obj.uvs.push_back({ 1.0f, 1.0f });
	obj.uvs.push_back({ 0.0f, 1.0f });
	obj.uvs.push_back({ 0.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 1.0f });
	obj.uvs.push_back({ 0.0f, 1.0f });
	obj.uvs.push_back({ 0.0f, 1.0f });
	obj.uvs.push_back({ 0.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 0.0f, 1.0f });
	obj.uvs.push_back({ 1.0f, 1.0f });
	obj.uvs.push_back({ 0.0f, 1.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 0.0f, 0.0f });
	obj.uvs.push_back({ 0.0f, 1.0f });
	obj.uvs.push_back({ 1.0f, 1.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 0.0f, 0.0f });
	obj.uvs.push_back({ 0.0f, 1.0f });
	obj.uvs.push_back({ 0.0f, 1.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 1.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 0.0f, 1.0f });
	obj.uvs.push_back({ 0.0f, 0.0f });

	obj.vertices.push_back({-size.x / 2.f, -size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f,  size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f, -size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f,  size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f, -size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f,  size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f, -size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f, -size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f,  size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f,  size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f,  size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f, -size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f,  size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f,  size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f, -size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f, -size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f, -size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f,  size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f,  size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f, -size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f,  size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f, -size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f,  size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f, -size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f, -size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f, -size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f, -size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f, -size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f, -size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f, -size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f,  size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f,  size.y / 2.f , size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f,  size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({ size.x / 2.f,  size.y / 2.f,  size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f,  size.y / 2.f, -size.z / 2.f});
	obj.vertices.push_back({-size.x / 2.f,  size.y / 2.f,  size.z / 2.f});

	return obj;
}

Object_File Object_File::tetraedre(Vector3f size) noexcept {
	Object_File obj;
	obj.min = size / -2;
	obj.max = size / +2;

	obj.vertices.push_back({ 0, 0, 0.5f * size.z });
	obj.vertices.push_back({ -0.5f * size.x, -0.5f * size.y, -0.5f * size.z });
	obj.vertices.push_back({ +0.5f * size.x, -0.5f * size.y, -0.5f * size.z });

	obj.uvs.push_back({ 0.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 0.5f, 1.0f });

	obj.vertices.push_back({ 0, 0, 0.5f * size.z });
	obj.vertices.push_back({ +0.5f * size.x, -0.5f * size.y, -0.5f * size.z });
	obj.vertices.push_back({ 0, +0.5f * size.y, -0.5f * size.z });

	obj.uvs.push_back({ 0.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 0.5f, 1.0f });

	obj.vertices.push_back({ 0, 0, 0.5f * size.z });
	obj.vertices.push_back({ -0.5f * size.x, -0.5f * size.y, -0.5f * size.z });
	obj.vertices.push_back({ 0, +0.5f * size.y, -0.5f * size.z });

	obj.uvs.push_back({ 0.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 0.5f, 1.0f });

	obj.vertices.push_back({ 0, +0.5f * size.y, -0.5f * size.z });
	obj.vertices.push_back({ -0.5f * size.x, -0.5f * size.y, -0.5f * size.z });
	obj.vertices.push_back({ +0.5f * size.x, -0.5f * size.y, -0.5f * size.z });

	obj.uvs.push_back({ 0.0f, 0.0f });
	obj.uvs.push_back({ 1.0f, 0.0f });
	obj.uvs.push_back({ 0.5f, 1.0f });

	return obj;
}
