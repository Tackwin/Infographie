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
#include "UI/Transform.hpp"
#include "UI/Texture.hpp"
#include "UI/Illumination.hpp"
#include "UI/Cameras.hpp"
#include "Scene/Image.hpp"
#include "Scene/Model.hpp"
#include "Scene/Widget.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Canvas.hpp"
#include "Scene/CubeMap.hpp"
#include "Scene/Primitive.hpp"
#include "Managers/AssetsManager.hpp"
#include "Managers/InputsManager.hpp"
#include "Utils/TimeInfo.hpp"
#include "Utils/Logs.hpp"

#include "Graphic/FrameBuffer.hpp"

// Turns out that sfml (wich we link statically) already implement stb_image
// so we don't do taht otherwise we would get duplicate symbol.
// #define STB_IMAGE_IMPLEMENTATION
// #include "Files/stb_image.h"

static bool Show_Render_Debug{ false };

void load_textures() noexcept;
void load_objects() noexcept;
void load_shaders() noexcept;

void construct_managers() noexcept;
void destroy_managers() noexcept;

void update(
	Widget3& root,
	Images_Settings& img_settings,
	Drawing_Settings& draw_settings,
	Transform_Settings& tran_settings,
	Geometries_Settings& geo_settings,
	Texture_Settings& tex_settings,
	Camera_Settings& cam_settings,
	Illumination_Settings& ill_settings,
	float dt
) noexcept;

void render(
	Widget3& root,
	Texture_Settings& tex_settings,
	Illumination_Settings& ill_settings,
	Texture_Buffer& texture_target,
	sf::RenderTexture& sf_texture_target,
	sf::RenderTarget& target,
	std::optional<std::filesystem::path> screenshot
) noexcept;

void render_postprocessing(
	Texture_Settings& settings, const sf::Texture& texture, sf::RenderTarget& target
) noexcept;

void update_debug_ui() noexcept;

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
	context_settings.majorVersion = 4;
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
	push_cursor(sf::Cursor::Arrow);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		(void)getchar();
		return -1;
	}

	if (glDebugMessageControl != NULL) {
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallback((GLDEBUGPROCARB)Common::verbose_opengl_error, NULL);
	}

	ImGui::SFML::Init(Window_Info.window);

	sf::RenderTexture sf_render_texture;
	sf_render_texture.create(UNROLL_2(Window_Info.size));

	Texture_Buffer render_texture(Window_Info.size);

	Images_Settings img_settings;
	Camera_Settings cam_settings;
	Texture_Settings tex_settings;
	Drawing_Settings draw_settings;
	Transform_Settings tran_settings;
	Geometries_Settings geo_settings;
	Illumination_Settings ill_settings;

	std::shared_mutex function_from_another_thread_mutex;
	std::vector<std::function<void(void)>> function_from_another_thread;

	Widget3 scene_root;

	auto add_cam_to_root = [](Widget3& root) -> Camera& {
		auto& camera = *root.make_child<Camera>();
		camera.set_viewport({ {0, 0}, {Window_Info.size.x, Window_Info.size.y } });
		camera.set_perspective(
			70 / (float)RAD_2_DEG, (float)Window_Info.size.x / (float)Window_Info.size.y, 500, 1
		);
		camera.set_global_position({ 0, 0, -10 });
		camera.look_at({ 0, 0, 0 }, { 0, 1, 0 });
		// it's ok to render from the parent all it's going to do is that the camera will try to call
		// it's propagate_opengl_render method. But a camera doesn't render anything, it's just
		// a simple class to hold the different transformation matrix. So it's going to be a no op
		// and pass to his siblings.
		camera.render_from(&root);
		return camera;
	};

	//auto& camera2 = *scene_root.make_child<Camera>();
	//camera2.set_viewport({ { Window_Info.size.x / 2, 0 }, { Window_Info.size.x / 2, 1 * Window_Info.size.y } });
	//camera2.set_perspective(
	//	70 / (float)RAD_2_DEG, (float)Window_Info.size.x / (float)Window_Info.size.y, 500, 1
	//);
	//camera2.set_global_position({ 0, 0, -10 });
	//camera2.look_at({ 0, 0, 0 }, { 0, 1, 0 });
	//camera2.render_from(&scene_root);

	img_settings.root = &scene_root;
	geo_settings.root = &scene_root;
	tex_settings.root = &scene_root;
	cam_settings.root = &scene_root;
	ill_settings.root = &scene_root;
	draw_settings.root = &scene_root;
	tran_settings.root = &scene_root;

	auto& main_cam = add_cam_to_root(scene_root);
	main_cam.set_input_active(true);
	main_cam.set_name("Main Camera");
	cam_settings.camera_ids.push_back(main_cam.get_uuid());

	//cam_settings.camera_ids.push_back(camera2.get_uuid());
	{
	img_settings.import_images_callback.push_back([&](const std::filesystem::path& path) {
		if (!AM->load_texture(path.generic_string(), path)) return;
		auto& texture = AM->get_texture(path.generic_string());
		auto img_widget = scene_root.make_child<Image>(texture);
		img_widget->set_global_position((Vector2f)Window_Info.size / 2);
		if (!img_widget->size_ok_for_sampling()) {
			Log.push("Please note that the sampling feature take sample of 30 px from an image\n\
				so you won't be able to sample this image");
			Log.show = true;
		}
		img_settings.images_widget_id.push_back(img_widget->get_uuid());
	});
	img_settings.create_images_callback.push_back([&](const sf::Image& image){
		static size_t Counter{ 0 };
		auto& texture = AM->create_texture(std::to_string(Counter++) + "_created_image___");
		if (!texture.loadFromImage(image)) {
			Log.push("Problem creating the texture from the sampled image.");
			return;
		}
		auto img_widget = scene_root.make_child<Image>(texture);
		img_widget->set_global_position((Vector2f)Window_Info.size / 2);
		img_settings.images_widget_id.push_back(img_widget->get_uuid());
	});
	draw_settings.add_canvas_callback.push_back([&](Vector2u size) {
		auto canvas_widget = scene_root.make_child<Canvas>(draw_settings);
		canvas_widget->set_size(size);
		canvas_widget->set_global_position((Vector2f)Window_Info.size / 2);
		draw_settings.canvases_widget_id.push_back(canvas_widget->get_uuid());
	});
	draw_settings.add_primitive_callback.push_back([&](std::unique_ptr<sf::Shape> shape) {
		auto primitive_widget = scene_root.make_child<Primitive>(std::move(shape));
		primitive_widget->set_global_position((Vector2f)Window_Info.size / 2);
		draw_settings.primitives_widget_id.push_back(primitive_widget->get_uuid());
	});
	draw_settings.add_complex_primitive_callback.push_back(
		[&](std::unique_ptr<Complex_Shape> shape) {
			auto primitive_widget = scene_root.make_child<Primitive>(std::move(shape));
			primitive_widget->set_global_position((Vector2f)Window_Info.size / 2);
			draw_settings.primitives_widget_id.push_back(primitive_widget->get_uuid());
		}
	);
	geo_settings.model_added_callback.push_back([&](const std::filesystem::path& path) {
		if (!AM->load_object_file(path.generic_string(), path)) return;
		std::lock_guard guard{ function_from_another_thread_mutex };
		function_from_another_thread.push_back([&, path] {
			std::lock_guard guard{ geo_settings.mutex };

			auto model_widget = scene_root.make_child<Model>();
			geo_settings.models_widget_id.push_back(model_widget->get_uuid());
			model_widget->set_object(AM->get_object_file(path.generic_string()));
			model_widget->set_shader(AM->get_shader("Deferred_Simple"));
			model_widget->set_select_shader(AM->get_shader("Uniform_Glow"));
		});
	});
	geo_settings.texture_callback.push_back(
		[&](Uuid_t id, std::filesystem::path path, Geometries_Settings::Texture_Type type) {
			if (!AM->load_texture(path.generic_string(), path)) return;
			auto model_widget = (Model*)scene_root.find_child(id);

			std::lock_guard guard{ function_from_another_thread_mutex };
			function_from_another_thread.push_back([model_widget, path, type] {
				using Enum = Geometries_Settings::Texture_Type;
				switch (type) {
				case Enum::Main :{
					model_widget->set_texture(AM->get_texture(path.generic_string()));
					break;
				}
				case Enum::Alpha :{
					model_widget->set_alpha_texture(AM->get_texture(path.generic_string()));
					break;
				}
				case Enum::Normal: {
					model_widget->set_normal_texture(AM->get_texture(path.generic_string()));
					break;
				}
				case Enum::Speculative: {
					model_widget->set_spec_texture(AM->get_texture(path.generic_string()));
					break;
				}
				case Enum::Metallic: {
					model_widget->set_metallic_texture(AM->get_texture(path.generic_string()));
					break;
				}
				case Enum::Roughness: {
					model_widget->set_roughness_texture(AM->get_texture(path.generic_string()));
					break;
				}
				case Enum::AO: {
					model_widget->set_ao_texture(AM->get_texture(path.generic_string()));
					break;
				}
				default: {
					assert("Should not happen !");
				}
				}

			});
		}
	);
	geo_settings.texture_generated_set_callback.push_back([&](Uuid_t id) {
		auto model_widget = (Model*)scene_root.find_child(id);
		model_widget->set_texture(tex_settings.gradient_texture);
	});
	geo_settings.spawn_object_callback.push_back(
		[&](const Object_File& obj) {
			auto model_widget = scene_root.make_child<Model>();
			geo_settings.models_widget_id.push_back(model_widget->get_uuid());
			model_widget->set_object_copy(obj);
			model_widget->set_shader(AM->get_shader("Deferred_Simple"));
			model_widget->set_select_shader(AM->get_shader("Uniform_Glow"));
		}
	);
	tex_settings.cubemap_added.push_back([&](std::filesystem::path path) {
		if (!std::filesystem::is_directory(path)) return;

		// i know i should stay away from manual memory management
		// but i don't want to make the render thread wait to load those images
		// and i'm not sure of the semantics of std::shared/unique_ptr just yet..
		sf::Image* data = new sf::Image[6];
		const auto texture_names =
			{ "right.png", "left.png", "top.png", "bot.png", "front.png", "back.png" };

		size_t i = 0;
		for (auto& x : texture_names) {
			if (!std::filesystem::is_regular_file(path / x)) {
				Log.push(std::string("Missing in the cube map folder: ") + x);
				return;
			}
			if (!data[i].loadFromFile((path / x).generic_string())) return;
			++i;
		}

		std::lock_guard guard{ function_from_another_thread_mutex };
		function_from_another_thread.push_back([&, p = path, d = data] {
			auto cubemap = new Cube_Map();
			scene_root.add_child(cubemap, -1);

			tex_settings.cubemap_ids.push_back(cubemap->get_uuid());
			cubemap->set_name((--p.end())->generic_string()); // <- that's code for the parent dir.
			cubemap->set_textures(std::move(d));

			// I hope this will eventually execute
			delete[] d;
		});
	});
	tex_settings.environment_added.push_back([&](std::filesystem::path path) {
		if (!std::filesystem::is_regular_file(path)) return;

		std::lock_guard guard{ function_from_another_thread_mutex };
		function_from_another_thread.push_back([&, p = path] {
			auto cubemap = new Cube_Map();
			scene_root.add_child(cubemap, 2); // We render last

			tex_settings.cubemap_ids.push_back(cubemap->get_uuid());
			cubemap->set_name(p.filename().generic_string());
			if (!cubemap->load_texture(p)) {
				Log.push("Couldn't load cubemap");
			}
		});
	});
	}
	sf::Clock dt_clock;
	float dt;
	while (Window_Info.window.isOpen()) {
		dt = dt_clock.restart().asSeconds();
		IM::update(Window_Info.window);
		if (!Window_Info.window.isOpen()) break;

		update(
			scene_root,
			img_settings,
			draw_settings,
			tran_settings,
			geo_settings,
			tex_settings,
			cam_settings,
			ill_settings,
			dt
		);

		{
			std::lock_guard guard{ function_from_another_thread_mutex };
			for (auto& f : function_from_another_thread) {
				f();
			}
			function_from_another_thread.clear();
		}

		std::optional<std::filesystem::path> screenshot;
		if (img_settings.take_screenshot) {
			img_settings.take_screenshot = false;
			auto n_files = std::filesystem::hard_link_count(img_settings.screenshot_directory);
			auto file_name =
				img_settings.screenshot_directory /
				("screenshot_" + std::to_string(n_files) + ".png");

			screenshot = file_name;
		}


		render(
			scene_root,
			tex_settings,
			ill_settings,
			render_texture,
			sf_render_texture,
			Window_Info.window,
			screenshot
		);

		Window_Info.window.display();
	}
	ImGui::SFML::Shutdown();
}

void update(
	Widget3& root,
	Images_Settings& img_settings,
	Drawing_Settings& draw_settings,
	Transform_Settings& tran_settings,
	Geometries_Settings& geo_settings,
	Texture_Settings& tex_settings,
	Camera_Settings& cam_settings,
	Illumination_Settings& ill_settings,
	float dt
) noexcept {
	static constexpr auto Help_Popup_Title = "Help - Hestia";

	ImGui::SFML::Update(Window_Info.window, sf::seconds(dt));
	root.propagate_input();

	list_all_logs_imgui();

	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);
	ImGui::BeginMenuBar();
	if (ImGui::MenuItem("Help")) {
		ImGui::OpenPopup(Help_Popup_Title);
	}

	if (ImGui::BeginPopup(Help_Popup_Title)) {
		ImGui::Text(R"__(
Projet Hestia



Ce projet est une application permettant de développer sa créativité en faisant une multitude de choses, on peut y :


importer des images et observer la composition de ses pixels grâce à des histogrammes de plusieurs domaines de couleurs
Créer des images et les exporter à partir de différents échantillons, provenant d'autres images ou créées à partir de palettes de couleurs.
Dessiner dans des endroits prévus à cet effet, de dimension variable, de couleur variable et avec des outils variables.
Faire apparaître des formes à partir d'une liste, de taille et couleur variable, avec des contours de taille et couleurs variables.
Charger des modèles 3D dans un espace que l'on peut parcourir en se déplaçant soit même et en tournant la caméra, et donner des textures à ces modèles, autant en transparence qu'en couleur.
Modifier des modèles 3D, en les déplaçant, tournant et en changeant leur échelles suivant différents axes, on peut revenir en arrière ou revenir en avant à notre guise.
Naviguer dans des cubemaps sur lequel on peut appliquer des filtres, filtres sur lesquels on peut agir, en modifiant leurs caractéristiques.
Créer des textures à appliquer sur des objets.
Gérer les différentes fonctionnalités de manière ordonnée, en pouvant dissimuler à tout moment tout élément et en pouvant aller le rechercher plus tard.
	)__");
		ImGui::EndPopup();
	}
	static sf::Cursor* help_cursor{ nullptr };
	auto hovered = ImGui::IsItemHovered();
	if (hovered && !help_cursor) {
		help_cursor = push_cursor(sf::Cursor::Help);
	} else if (!hovered && help_cursor) {
		pop_cursor(help_cursor);
		help_cursor = nullptr;
	}
	ImGui::EndMenuBar();
	ImGui::SetWindowPos({ 5, 5 });

	if (ImGui::CollapsingHeader("Image")) update_image_settings(img_settings);
	if (ImGui::CollapsingHeader("Drawing")) update_drawing_settings(draw_settings);
	if (ImGui::CollapsingHeader("Transform")) update_transform_settings(tran_settings);
	if (ImGui::CollapsingHeader("Geometries")) update_geometries_settings(geo_settings);
	if (ImGui::CollapsingHeader("Texture")) update_texture_settings(tex_settings);
	if (ImGui::CollapsingHeader("Camera")) update_camera_settings(cam_settings);
	if (ImGui::CollapsingHeader("Illumination")) update_illumination_settings(ill_settings);
	if (ImGui::CollapsingHeader("Debug")) update_debug_ui();

	root.propagate_update(dt);
	ImGui::End();
}

void render(
	Widget3& root,
	Texture_Settings& tex_settings,
	Illumination_Settings& ill_settings,
	Texture_Buffer& texture_target,
	sf::RenderTexture& sf_texture_target,
	sf::RenderTarget& target,
	std::optional<std::filesystem::path> screenshot
) noexcept {
	static sf::Clock dt_clock;
	Is_In_Sfml_Context = false;

	target.clear({ 0, 0, 0, 255 });

	//glDisable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_TEXTURE_2D);
	//glEnable(GL_LIGHTING);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	texture_target.set_active();
	texture_target.clear({ 0, 0, 0, 1 });

	auto& shader_light = AM->get_shader("Deferred_Light");
	shader_light.setUniform("ambient_strength", ill_settings.ambient.strength);
	shader_light.setUniform("ambient_color", sf::Vector3f{ UNROLL_3(ill_settings.ambient.color) });

	for (size_t i = 0; i < ill_settings.directionals.size(); ++i) {
		auto& x = ill_settings.directionals[i];

		auto str = "light_dirs[" + std::to_string(i) + "].";

		shader_light.setUniform(str + "dir", sf::Vector3f{ UNROLL_3(x.dir) } );
		shader_light.setUniform(str + "color", sf::Vector3f{ UNROLL_3(x.color) });
		shader_light.setUniform(str + "strength", x.strength);
	}

	ImGui::Begin("Debug render info", &Show_Render_Debug);
	ImGui::Text("dt: %3.3f ms", dt_clock.restart().asSeconds() * 1000.f);
	root.propagate_render(texture_target);
	sf_texture_target.setActive(true);
	sf_texture_target.clear({ 0, 0, 0, 0 });
	root.propagate_render(sf_texture_target);
	ImGui::End();


	target.setActive(true);

	Is_In_Sfml_Context = true;
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	target.resetGLStates();
	if (screenshot) {
		sf::RenderTexture render_texture;
		render_texture.create(UNROLL_2(Window_Info.size));
		
		root.propagate_render(render_texture);
	
		render_postprocessing(tex_settings, render_texture.getTexture(), render_texture);
		ImGui::SFML::Render(render_texture);

		render_texture.display();
		render_texture.getTexture().copyToImage().saveToFile(screenshot->generic_string());
	}
	render_postprocessing(tex_settings, texture_target.get_sfml_texture(), target);
	render_postprocessing(tex_settings, sf_texture_target.getTexture(), target);
	ImGui::SFML::Render(target);
	glPopAttrib();
}

void render_postprocessing(
	Texture_Settings& settings, const sf::Texture& texture, sf::RenderTarget& target
) noexcept {
	auto& shader = AM->get_shader(Texture_Settings::Tone_String[(int)settings.current_tone]);
	shader.setUniform("texture", texture);

	switch (settings.current_tone)
	{
	case Texture_Settings::Tone::Blur:
		shader.setUniform("blur_radius", settings.blur_radius);
		break;
	case Texture_Settings::Tone::BW:
		shader.setUniform("leakage", settings.bw_leakage);
		break;
	case Texture_Settings::Tone::Edge:
		shader.setUniform("edge_threshold", settings.edge_threshold);
		break;
	case Texture_Settings::Tone::Sepia: {
		sf::Glsl::Vec3 red;
		sf::Glsl::Vec3 green;
		sf::Glsl::Vec3 blue;
		red.x = settings.sepia_weights.x.x;
		red.y = settings.sepia_weights.x.y;
		red.z = settings.sepia_weights.x.z;

		green.x = settings.sepia_weights.y.x;
		green.y = settings.sepia_weights.y.y;
		green.z = settings.sepia_weights.y.z;

		blue.x = settings.sepia_weights.z.x;
		blue.y = settings.sepia_weights.z.y;
		blue.z = settings.sepia_weights.z.z;

		shader.setUniform("sepia_red", red);
		shader.setUniform("sepia_green", green);
		shader.setUniform("sepia_blue", blue);
		break;
	}
	default:
		break;
	}

	sf::Sprite sprite{ texture };
	sprite.setScale(1, -1);
	sprite.setPosition(0, (float)Window_Info.size.y);
	target.draw(sprite, &shader);
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
	AM->load_texture("White", "res/White.png");
	AM->load_texture("DT_Square", "res/DT_Square.png");
	AM->load_texture("DT_Fill", "res/DT_Fill.png");
	AM->load_texture("Cube_Icon", "res/cube_icon.png");
	AM->load_texture("Tetra_Icon", "res/tetraèdre_icon.png");
	AM->load_texture("PT_Polygon", "res/PT_Polygon.png");
	AM->load_texture("PT_Arrow", "res/PT_Arrow.png");
	AM->load_texture("PT_Star", "res/PT_Star.png");
	AM->load_texture("PT_Heart", "res/PT_Heart.png");
	AM->load_texture("PT_Rect", "res/PT_Rect.png");
}

void load_objects() noexcept {}

void load_shaders() noexcept {
	AM->load_shader(
		Texture_Settings::Tone_String[(int)Texture_Settings::Tone::Identity],
		"res/shaders/Identity.vertex",
		"res/shaders/Identity.fragment"
	);
	AM->load_shader(
		Texture_Settings::Tone_String[(int)Texture_Settings::Tone::BW],
		"res/shaders/Identity.vertex",
		"res/shaders/Black_And_White.fragment"
	);
	AM->load_shader(
		Texture_Settings::Tone_String[(int)Texture_Settings::Tone::Sepia],
		"res/shaders/Identity.vertex",
		"res/shaders/Sepia.fragment"
	);
	AM->load_shader(
		Texture_Settings::Tone_String[(int)Texture_Settings::Tone::Blur],
		"res/shaders/Identity.vertex",
		"res/shaders/Blur.fragment"
	);
	AM->load_shader(
		Texture_Settings::Tone_String[(int)Texture_Settings::Tone::Edge],
		"res/shaders/Identity.vertex",
		"res/shaders/Edge.fragment"
	);
	AM->load_shader("Simple", "res/shaders/simple.vertex", "res/shaders/simple.fragment");
	AM->load_shader("Skybox", "res/shaders/Skybox.vertex", "res/shaders/Skybox.fragment");
	AM->load_shader(
		"Uniform_Glow",
		"res/shaders/uniform_glow.vertex",
		"res/shaders/uniform_glow.fragment",
		"res/shaders/Implode.geometry"
	);
	AM->load_shader(
		"Explode",
		"res/shaders/uniform_glow.vertex",
		"res/shaders/uniform_glow.fragment",
		"res/shaders/Explode.geometry"
	);
	AM->load_shader(
		"Deferred_Simple",
		"res/shaders/Simple_Deferred.vertex",
		"res/shaders/Simple_Deferred.fragment"
	);
	AM->load_shader(
		"Deferred_Light",
		"res/shaders/Light_Deferred.vertex",
		"res/shaders/Light_Deferred.fragment"
	);
	AM->load_shader(
		"Deferred_Debug",
		"res/shaders/FBO_Debug.vertex",
		"res/shaders/FBO_Debug.fragment"
	);
	AM->load_shader("Light_Box", "res/shaders/Light_Box.vertex", "res/shaders/Light_Box.fragment");
	AM->load_shader("HDR", "res/shaders/HDR.vertex", "res/shaders/HDR.fragment");
}

void update_debug_ui() noexcept {
	thread_local bool show_demo_window{ false };
	if (!Show_Render_Debug && ImGui::Button("Show render debug")) Show_Render_Debug = true;

	if (!Log.data.empty() && ImGui::Button("Show logs")) Log.show = true;
	ImGui::Checkbox("Demo", &show_demo_window);
	if (show_demo_window) ImGui::ShowDemoWindow();
}
