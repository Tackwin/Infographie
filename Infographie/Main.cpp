#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "Window.hpp"

#include "Math/Matrix.hpp"

#include "UI/Images.hpp"
#include "UI/Drawings.hpp"
#include "UI/Geometries.hpp"
#include "Scene/Widget.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Image.hpp"
#include "Scene/Model.hpp"
#include "Scene/Canvas.hpp"
#include "Managers/AssetsManager.hpp"
#include "Managers/InputsManager.hpp"
#include "Utils/TimeInfo.hpp"
#include "Utils/Logs.hpp"

void load_textures() noexcept;
void load_objects() noexcept;
void load_shaders() noexcept;

void construct_managers() noexcept;
void destroy_managers() noexcept;

void update(
	Widget3& root,
	Camera& camera,
	Images_Settings& img_settings,
	Drawing_Settings& draw_settings,
	Geometries_Settings& geo_settings,
	float dt
) noexcept;
void render(Widget3& root,Camera& camera, sf::RenderTarget& target) noexcept;

void update_debug_ui(Widget3& root, Camera& camera) noexcept;

int main() {
	View_Matrix = new Matrix4f();
	Projection_Matrix = new Matrix4f();

	construct_managers();
	defer{ destroy_managers(); };

	load_textures();
	load_objects();
	load_shaders();

	details::Window_Struct::instance = new details::Window_Struct();
	defer{ delete details::Window_Struct::instance; };

	sf::ContextSettings context_settings;
	context_settings.depthBits = 24;
	context_settings.stencilBits = 8;
	context_settings.antialiasingLevel = 4;
	context_settings.majorVersion = 3;
	context_settings.minorVersion = 3;

	Window_Info.title = "Infographie";
	Window_Info.size = { 1600u, 900u };
	Window_Info.clear_color = { 0.2f, 0.2f, 0.2f };
	Window_Info.window.create(
		sf::VideoMode{ UNROLL_2_P(Window_Info.size, size_t) },
		Window_Info.title,
		sf::Style::Default,
		context_settings
	);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		return -1;
	}

	check_gl_error();

	ImGui::SFML::Init(Window_Info.window);
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);

	Images_Settings img_settings;
	Drawing_Settings draw_settings;
	Geometries_Settings geo_settings;

	std::shared_mutex function_from_another_thread_mutex;
	std::vector<std::function<void(void)>> function_from_another_thread;

	Widget3 scene_root;
	auto& camera = *scene_root.make_child<Camera>();
	camera.set_viewport({ 0, 0, UNROLL_2(Window_Info.size) });
	camera.set_perspective(
		90 / (float)RAD_2_DEG, (float)Window_Info.size.x / (float)Window_Info.size.y, 500, 1
	);
	camera.set_global_position({ 0, 0, -5 });
	camera.look_at({ 0, 0, 0 });
	// it's ok to render from the parent all it's going to do is that the camera will try to call
	// it's propagate_opengl_render method. But a camera doesn't render anything, it's just
	// a simple class to hold the different transformation matrix. So it's going to be a no op
	// and pass to his siblings.
	camera.render_from(&scene_root);
	

	img_settings.root = &scene_root;
	draw_settings.root = &scene_root;
	geo_settings.root = &scene_root;

	img_settings.import_images_callback.push_back([&](const std::filesystem::path& path) {
		if (!AM->load_texture(path.generic_string(), path)) return;
		auto img_widget = scene_root.make_child<Image>(AM->get_texture(path.generic_string()));
		img_settings.images_widget_id.push_back(img_widget->get_uuid());
	});

	draw_settings.add_canvas_callback.push_back([&](Vector2u size) {
		auto canvas_widget = scene_root.make_child<Canvas>(draw_settings);
		canvas_widget->set_size(size);
		draw_settings.canvases_widget_id.push_back(canvas_widget->get_uuid());
	});

	geo_settings.model_added_callback.push_back([&](const std::filesystem::path& path) {
		if (!AM->load_object_file(path.generic_string(), path)) return;
		std::lock_guard{ function_from_another_thread_mutex };
		function_from_another_thread.push_back([&, path] {
			std::lock_guard{ geo_settings.mutex };

			auto model_widget = scene_root.make_child<Model>();
			geo_settings.models_widget_id.push_back(model_widget->get_uuid());
			model_widget->set_object(AM->get_object_file(path.generic_string()));
			model_widget->set_shader(AM->get_shader("Simple"));
			model_widget->set_select_shader(AM->get_shader("Uniform_Glow"));
		});
	});

	geo_settings.texture_added_callback.push_back(
		[&](Uuid_t id, const std::filesystem::path& path) {
			if (!AM->load_texture(path.generic_string(), path)) return;
			auto model_widget = (Model*)scene_root.find_child(id);

			std::lock_guard{ function_from_another_thread_mutex };
			function_from_another_thread.push_back([model_widget, path] {
				model_widget->set_texture(AM->get_texture(path.generic_string()));
			});
		}
	);

	geo_settings.spawn_object_callback.push_back(
		[&](const Object_File& obj) {
			auto model_widget = scene_root.make_child<Model>();
			geo_settings.models_widget_id.push_back(model_widget->get_uuid());
			model_widget->set_object_copy(obj);
			model_widget->set_shader(AM->get_shader("Simple"));
			model_widget->set_select_shader(AM->get_shader("Uniform_Glow"));
		}
	);


	sf::Clock dt_clock;
	float dt;
	while (Window_Info.window.isOpen()) {
		dt = dt_clock.restart().asSeconds();
		IM::update(Window_Info.window);
		if (!Window_Info.window.isOpen()) break;

		glClearColor(UNROLL_3(Window_Info.clear_color), 1); check_gl_error();
		glClearDepth(0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); check_gl_error();

		{
		std::lock_guard{ function_from_another_thread_mutex };
		for (auto& f : function_from_another_thread) {
			f();
		}
		function_from_another_thread.clear();
		}

		update(scene_root, camera, img_settings, draw_settings, geo_settings, dt);

		render(scene_root, camera, Window_Info.window);

		Window_Info.window.display();
	}
	ImGui::SFML::Shutdown();
}

void update(
	Widget3& root,
	Camera& camera,
	Images_Settings& img_settings,
	Drawing_Settings& draw_settings,
	Geometries_Settings& geo_settings,
	float dt
) noexcept {

	ImGui::SFML::Update(Window_Info.window, sf::seconds(dt));
	root.propagate_input();

	list_all_logs_imgui();

	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove);
	defer{ ImGui::End(); };

	ImGui::SetWindowPos({ 5, 5 });

	if (ImGui::CollapsingHeader("Image")) update_image_settings(img_settings);
	if (ImGui::CollapsingHeader("Drawing")) update_drawing_settings(draw_settings);
	if (ImGui::CollapsingHeader("Geometries")) update_geometries_settings(geo_settings);
	if (ImGui::CollapsingHeader("Debug")) update_debug_ui(root, camera);

	root.propagate_update(dt);

	if (img_settings.take_screenshot) {
		defer{ img_settings.take_screenshot = false; };

		sf::RenderTexture render_texture;
		render_texture.create(UNROLL_2(Window_Info.size));
		render_texture.clear();
		render(root, camera, render_texture);
		render_texture.display();

		auto number_of_files = std::filesystem::hard_link_count(img_settings.screenshot_directory);
		auto file_name =
			img_settings.screenshot_directory /
			("screenshot_" + std::to_string(number_of_files) + ".png");

		render_texture.getTexture().copyToImage().saveToFile(file_name.generic_string());
	}
}

void render(Widget3& root, Camera& camera, sf::RenderTarget& target) noexcept {
	glPopAttrib();

	Is_In_Sfml_Context = true;
	Window_Info.window.pushGLStates();
	defer {
		Window_Info.window.popGLStates();
		Is_In_Sfml_Context = false;

		glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_SCISSOR_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);

		glFrontFace(GL_CW);
		glCullFace(GL_FRONT);
		glDepthFunc(GL_GREATER);
		Window_Info.window.setActive();
	};
	check_gl_error();

	Window_Info.active_camera = &camera;
	root.propagate_opengl_render();
	Window_Info.active_camera = nullptr;

	ImGui::SFML::Render(target);
	check_gl_error();
	root.propagate_render(target);
	check_gl_error();
}


void construct_managers() noexcept {
	AM = new Assets_Manager();
}

void destroy_managers() noexcept {
	delete AM;
}

void load_textures() noexcept {
	AM->load_texture("Primitives_Tool", "res/Primitives_Tool.png");
	AM->load_texture("Drawings_Tool", "res/Drawings_Tool.png");
	AM->load_texture("DT_Circle", "res/DT_Circle.png");
	AM->load_texture("DT_Line", "res/DT_Line.png");
	AM->load_texture("DT_Square", "res/DT_Square.png");
	AM->load_texture("DT_Fill", "res/DT_Fill.png");
	AM->load_texture("Cube_Icon", "res/cube_icon.png");
	AM->load_texture("Tetra_Icon", "res/tetraèdre_icon.png");
}

void load_objects() noexcept {
	AM->load_object_file("Bill", "res/models/BillCipher.obj");
}

void load_shaders() noexcept {
	AM->load_shader("Simple", "res/shaders/simple.vertex", "res/shaders/simple.fragment");
	AM->load_shader(
		"Uniform_Glow", "res/shaders/uniform_glow.vertex", "res/shaders/uniform_glow.fragment"
	);
}

void update_debug_ui(Widget3&, Camera& camera) noexcept {
	static float rotation = 0;
	static float cam_speed = 5;
	static float cam_fov = 90;
	static Vector3f light_pos{};
	static bool use_identity{ false };

	ImGui::DragFloat("Rotate", &rotation, 0.02f, 0, 2 * PIf);
	ImGui::DragFloat("Camera speed", &cam_speed, 0.1f, 0, 10);
	ImGui::DragFloat("Camera fov", &cam_fov, 1, 0, 180);
	ImGui::Checkbox("Use Identity", &use_identity);

	camera.set_perspective(
		cam_fov / (float)RAD_2_DEG, (float)Window_Info.size.x / (float)Window_Info.size.y, 500, 1
	);

	ImGui::Columns(3);
	ImGui::DragFloat("Light X", &light_pos.x, 0.02f, -2, 2); ImGui::NextColumn();
	ImGui::DragFloat("Light Y", &light_pos.y, 0.02f, -2, 2); ImGui::NextColumn();
	ImGui::DragFloat("Light Z", &light_pos.z, 0.02f, -2, 2);

	ImGui::Columns(1);
	ImGui::Text("x: %.3f y: %.3f z: %.3f",
		camera.get_global_position3().x,
		camera.get_global_position3().y,
		camera.get_global_position3().z
	);

	debug_values["Rotate"] = rotation;
	debug_values["Light_X"] = light_pos.x;
	debug_values["Light_Y"] = light_pos.y;
	debug_values["Light_Z"] = light_pos.z;
	debug_values["Camera_Speed"] = cam_speed;
	debug_values["Use_Identity"] = use_identity;

	if (!Log.data.empty() && ImGui::Button("Show logs")) Log.show = true;
}
