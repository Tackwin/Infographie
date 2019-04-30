#include "RayTracing.hpp"

#include "Scene/Camera.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include "Math/Ray.hpp"
#include "Math/algorithms.hpp"
#include "Utils/Logs.hpp"
#include "OS/OpenFile.hpp"

#include <SFML/Graphics.hpp>

struct Scene_Opts {
	struct Ball {
		float r{ 1.f };

		Vector3f pos{1, 1, 0.5f};

		Vector3f surface_color{ 1, 0, 1 };
		Vector3f emission_color{ 0, 0, 0 };
		float transparency{ 0 };
		float reflection{ 0 };
	};
	
	std::vector<Ball> balls;

	float fov{ 60.f };

	size_t max_depth{ 5 };

	Vector2u resolution{ 1600, 900 };
	Vector3f back_color{ 0, 0, 0 };

	float exposure{ 1.f };
	float gamma{ 2.2f };
};

sf::Image render_scene(Scene_Opts opts) noexcept;
Scene_Opts default_scene() noexcept;

void update_ray_tracing_settings(Ray_Tracing_Settings& settings) noexcept {
	static Scene_Opts opts = default_scene();

	int x = settings.blur_radius;
	ImGui::DragInt("Blur radius", &x, 1, 1, 16);
	settings.blur_radius = x;

	ImGui::DragFloat("SSAO radius", &settings.ssao_radius, 0.001f, 0.f, 1.f);
	ImGui::DragFloat("SSAO bias", &settings.ssao_bias, 0.001f, 0.f, 1.f);

	x = settings.kernel_sample_count;
	ImGui::DragInt("Kernel sample count", &x, 1, 32, 256);
	settings.kernel_sample_count = x;

	ImGui::DragFloat("Noise scale", &settings.noise_scale, 0.5f, 1.f, 10.f);

	ImGui::Separator();

	ImGui::Text("CPU Ray Trace !");
	ImGui::SameLine();
	if (ImGui::Button("Calculate !")) {
		open_dir_async([](std::optional<std::filesystem::path> path) {
			if (!path) {
				Log.push("Please select a directory.");
				return;
			}
			auto img = render_scene(opts);
			img.saveToFile((*path / "ray tracing result.png").generic_string());
			Log.push("Ray trace available.");
		});
	}

	if (ImGui::Button("Add Ball")) {
		opts.balls.push_back({});
	}

	x = (int)opts.max_depth;
	Vector2i r = (Vector2i)opts.resolution;
	ImGui::DragFloat("FOV", &opts.fov, 1, 30, 100);
	ImGui::DragFloat("Gamma", &opts.gamma, 0.1f, 1.f, 3.f);
	ImGui::DragFloat("Exposure", &opts.exposure, 0.02f, 0.f, 1.f);
	ImGui::DragInt("Recursion Depth", &x, 1, 0);
	ImGui::DragInt2("Resolution", &r.x);
	ImGui::ColorEdit3("Background", &opts.back_color.x);
	opts.max_depth = (size_t)std::max(0, x);
	opts.resolution = { (size_t)std::max(r.x, 0), (size_t)std::max(r.y, 0) };

	if (ImGui::CollapsingHeader("Balls")) {
		ImGui::PushID("Balls");
		defer{ ImGui::PopID(); };

		for (size_t i = 0; i < opts.balls.size(); ++i) {
			ImGui::PushID(i);
			defer{ ImGui::PopID(); };

			ImGui::DragFloat("Radius", &opts.balls[i].r, 0.1f, 0.f);
			ImGui::DragFloat3("Position", &opts.balls[i].pos.x);
			ImGui::ColorEdit3("Surface Color", &opts.balls[i].surface_color.x);
			ImGui::ColorEdit3("Emissive Color", &opts.balls[i].emission_color.x);
			ImGui::DragFloat("Transparency", &opts.balls[i].transparency, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("Reflection", &opts.balls[i].reflection, 0.01f, 0.f, 1.f);
		}
	}
	if (!settings.root) return;

	settings.root->for_every_childs([&](Widget * w) {
		if (auto* c = dynamic_cast<Camera*>(w); c) {
			c->ray_tracing_settings = settings;
		}
	});
}


Vector3f trace(const Scene_Opts& opts, Ray3f ray, size_t current_depth) noexcept;

sf::Image render_scene(Scene_Opts opts) noexcept {
	sf::Image img;
	img.create(opts.resolution.x, opts.resolution.y, (sf::Color)opts.back_color);

	float aspect_ratio = opts.resolution.x / (float)opts.resolution.y;
	float angle = tanf(PIf * 0.5f * opts.fov / 180.f);

	for (size_t x = 0; x < opts.resolution.x; ++x) {
		for (size_t y = 0; y < opts.resolution.y; ++y) {
			Ray3f ray;
			ray.dir.x = (float)((2 * ((x + 0.5) / opts.resolution.x) - 1) * angle * aspect_ratio);
			ray.dir.y = (float)((1 - 2 * ((y + 0.5) / opts.resolution.y)) * angle);
			ray.dir.z = -1;

			ray.pos = { 0, 5, 2 };

			ray.dir.normalize();

			auto color = trace(opts, ray, 0);

			auto f = [exposure = opts.exposure, gamma = opts.gamma](float x) {
				return std::powf(1 - std::expf(-x * exposure), 1 / gamma);
			};

			color.x = f(color.x);
			color.y = f(color.y);
			color.z = f(color.z);

			img.setPixel(x, y, (sf::Color)color);
		}
	}

	return img;
}

Vector3f trace(const Scene_Opts& scene, Ray3f ray, size_t current_depth) noexcept {

	std::optional<float> t_near;
	const Scene_Opts::Ball* sphere = NULL;

	// find intersection of this ray with the sphere in the scene
	for (auto& x : scene.balls) {
		if (auto t = ray_sphere(ray, x.pos, x.r); t) {
			if (t->x < 0) t->x = t->y;
			if (!t_near || t->x < *t_near) {
				t_near = t->x;
				sphere = &x;
			}
		}
	}

	// if there's no intersection return black or background color
	if (!sphere) return scene.back_color;
	
	// color of the ray/surfaceof the object intersected by the ray 
	Vector3f surface_color = { 0, 0, 0 };
	
	// point of intersection 
	Vector3f phit = ray.pos + ray.dir * (*t_near);

	float attenuation = 1.f;

	// normal at the intersection point 
	Vector3f nhit = phit - sphere->pos;

	// normalize normal direction 
	nhit.normalize();

	// If the normal and the view direction are not opposite to each other
	// reverse the normal direction. That also means we are inside the sphere so set
	// the inside bool to true. Finally reverse the sign of IdotN which we want
	// positive.
	float bias = 1e-4f; // add some bias to the point from which we will be tracing 

	bool inside = false;
	
	if (ray.dir.dot(nhit) > 0) {
		nhit = -1 * nhit;
		inside = true;
	}
	
	if ((sphere->transparency > 0 || sphere->reflection > 0) && current_depth < scene.max_depth) {
		float facingratio = -ray.dir.dot(nhit);

		// change the mix value to tweak the effect
		float fresneleffect = xstd::lerp(0.1f, 1.f, powf(1 - facingratio, 3));
		Ray3f new_ray;

		// compute reflection direction (not need to normalize because all vectors
		// are already normalized)
		new_ray.dir = ray.dir - nhit * 2 * ray.dir.dot(nhit);
		new_ray.dir.normalize();
		new_ray.pos = phit + nhit * bias;

		Vector3f reflection = trace(scene, ray, current_depth + 1);

		Vector3f refraction = { 0, 0, 0 };

		// if the sphere is also transparent compute refraction ray (transmission)
		if (sphere->transparency) {
			float ior = 1.1f;
			float eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface? 
			float cosi = -nhit.dot(ray.dir);
			float k = 1 - eta * eta * (1 - cosi * cosi);

			new_ray.dir = ray.dir * eta + nhit * (eta * cosi - sqrt(k));
			new_ray.dir.normalize();
			new_ray.pos = phit - nhit * bias;

			refraction = trace(scene, new_ray, current_depth + 1);
		}
		// the result is a mix of reflection and refraction (if the sphere is transparent)
		surface_color = (
			reflection * fresneleffect +
			sphere->transparency * refraction* (1 - fresneleffect)
		);
		surface_color.x *= sphere->surface_color.x;
		surface_color.y *= sphere->surface_color.y;
		surface_color.z *= sphere->surface_color.z;
	
		return surface_color + sphere->emission_color;
	}

	// it's a diffuse object, no need to raytrace any further
	for (size_t i = 0; i < scene.balls.size(); ++i) {
		auto& ball = scene.balls[i];

		if (ball.emission_color.x > 0) {
			// this is a light
			Vector3f transmission = { 1, 1, 1 };
			Vector3f lightDirection = ball.pos - phit;

			lightDirection.normalize();

			for (size_t j = 0; j < scene.balls.size(); ++j) {
				if (i != j) {
					Ray3f new_ray;
					new_ray.pos = phit + nhit * bias;
					new_ray.dir = lightDirection;

					if (ray_sphere(new_ray, scene.balls[j].pos, scene.balls[j].r)) {
						transmission = { 0, 0, 0};
						break;
					}
				}
			}

			Vector3f to_add = sphere->surface_color * std::max(0.f, nhit.dot(lightDirection));

			to_add.x *= transmission.x * ball.emission_color.x;
			to_add.y *= transmission.y * ball.emission_color.y;
			to_add.z *= transmission.z * ball.emission_color.z;

			surface_color += to_add;
		}
	}

	return surface_color + sphere->emission_color;
}

Scene_Opts default_scene() noexcept {
	Scene_Opts opts;
	Scene_Opts::Ball b;

	b.pos = Vector3f(0.f, -10004.f, -20.f);
	b.r = 10000;
	b.surface_color = { 0.20f, 0.20f, 0.20f };
	b.reflection = 0.f;
	b.transparency = 0.0f;

	opts.balls.push_back(b);

	b.pos = Vector3f(0.f, 0.f, -20.f);
	b.r = 4;
	b.surface_color = { 1.00f, 0.32f, 0.36f };
	b.reflection = 1;
	b.transparency = 0.5f;

	opts.balls.push_back(b);

	b.pos = Vector3f(5.f, -1.f, -15.f);
	b.r = 2;
	b.surface_color = { 0.90f, 0.76f, 0.46f };
	b.reflection = 1;
	b.transparency = 0.0f;

	opts.balls.push_back(b);

	b.pos = Vector3f(5.f, 0.f, -25.f);
	b.r = 3;
	b.surface_color = { 0.65f, 0.77f, 0.97f };

	opts.balls.push_back(b);

	b.pos = Vector3f(-5.5f, 0.f, -15.f);
	b.r = 3;
	b.surface_color = { 0.90f, 0.90f, 0.90f };

	opts.balls.push_back(b);

	// light

	b.pos = Vector3f(0.f, 20.f, -30.f);
	b.r = 3;
	b.surface_color = { 0.f, 0.f, 0.f };
	b.reflection = 0;
	b.emission_color = { 3.f, 3.f, 3.f };
	opts.balls.push_back(b);
	return opts;
}
