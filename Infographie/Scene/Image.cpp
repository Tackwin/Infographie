#include "Image.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

size_t Image::Total_N = 0;

Image::Image(sf::Texture& texture) noexcept {
	Total_N++;
	n = Total_N;
	sprite.setTexture(texture);
	image = texture.copyToImage();
}

void Image::update(float dt) noexcept {
	Widget::update(dt);
	if (!is_visible()) return;
	if (!open) return;

	ImGui::Begin(std::to_string(n).c_str(), &open);

	auto window_size = ImGui::GetWindowSize();
	auto image_size = Vector2f{
		(float)sprite.getTextureRect().width,
		(float)sprite.getTextureRect().height
	}.fit_into_preserve_ratio({ window_size.x * 0.8f, window_size.y / 2.f });

	auto image_pos = Vector2f{ ImGui::GetCursorPos() } +Vector2f{ ImGui::GetWindowPos() };
	ImGui::Image(sprite, image_size);

	ImGui::Checkbox("Histogramme", &histogram_open);

	// I seriously CAN't WAIT for full compile time reflection in this language, ffs everyone else
	// already has it in one form or another...
	static const char* const histogram_type_list[] = { "Grey Scale", "RGB Scale" };
	ImGui::ListBox(
		"Histgoram type", (int*)&histogram_type, histogram_type_list, (int)Histogram_Type::Count
	);

	if (histogram_open) update_histogram();
	update_echantillon();

	ImGui::SetWindowSize({
		std::max(ImGui::GetWindowSize().x, 100.f),
		std::max(ImGui::GetWindowSize().y, 100.f)
	});

	ImGui::End();

	set_position(image_pos);
	set_size(image_size);
}

void Image::update_echantillon() noexcept {
	ImGui::Separator();
	ImGui::Checkbox("Echantillon", &taking_echantillon);

	if (taking_echantillon) {
		ImGui::PushID("Echantillon");
		defer{ ImGui::PopID(); };

		auto max_x = get_size().x * (1 - 30.f / sprite.getTextureRect().width);
		auto max_y = get_size().y * (1 - 30.f / sprite.getTextureRect().height);

		ImGui::Columns(3);
		ImGui::PushID("Pos");
		ImGui::Text("Pos");
		ImGui::NextColumn();
		ImGui::DragFloat("##x", &echantillon.pos.x, 1, 0, max_x);
		ImGui::NextColumn();
		ImGui::DragFloat("##y", &echantillon.pos.y, 1, 0, max_y);
		ImGui::NextColumn();
		ImGui::PopID();
		ImGui::Columns(1);
	}

}

void Image::update_histogram() noexcept {
	if (histogram_type == Histogram_Type::Grey_Scale) {
		if (!grey_scale_histogram.cached) {
			grey_scale_histogram.cached = true;
			grey_scale_histogram.compute(*sprite.getTexture());
		}
		using Data_Type = decltype(grey_scale_histogram.data);
		ImGui::PlotHistogram(
			"Grey Scale",
			[](void* data, int idx) -> float {
				// We _know_ that data is of type &decltype(grey_scale_histogram.data)
				Data_Type& d = *reinterpret_cast<Data_Type*>(data);

				return (float)d[idx] / (float)d[256];
			},
			&grey_scale_histogram.data,
			255,
			0,
			nullptr,
			0,
			1.f,
			{0, 100}
		);
	}
	else if (histogram_type == Histogram_Type::RGB) {
		if (!rgb_scale_histogram.cached) {
			rgb_scale_histogram.cached = true;
			rgb_scale_histogram.compute(*sprite.getTexture());
		}

		// lots of repetition here but i don't feel like making a small abstraction would make
		// things clearer.
		using Red_Type = decltype(rgb_scale_histogram.reds);
		ImGui::PlotHistogram(
			"Red Scale",
			[](void* data, int idx) -> float {
				// We _know_ that data is of type &decltype(rgb_scale_histogram.reds)
				Red_Type& d = *reinterpret_cast<Red_Type*>(data);

				return (float)d[idx] / (float)d[256];
			},
			&rgb_scale_histogram.reds,
			255,
			0,
			nullptr,
			0,
			1.f,
			{ 0, 100 }
		);
		using Green_Type = decltype(rgb_scale_histogram.greens);
		ImGui::PlotHistogram(
			"Green Scale",
			[](void* data, int idx) -> float {
				// We _know_ that data is of type &decltype(rgb_scale_histogram.reds)
			Green_Type& d = *reinterpret_cast<Green_Type*>(data);

				return (float)d[idx] / (float)d[256];
			},
			&rgb_scale_histogram.greens,
			255,
			0,
			nullptr,
			0,
			1.f,
			{ 0, 100 }
		);
		using Blue_Type = decltype(rgb_scale_histogram.blues);
		ImGui::PlotHistogram(
			"Blue Scale",
			[](void* data, int idx) -> float {
				// We _know_ that data is of type &decltype(rgb_scale_histogram.reds)
				Blue_Type& d = *reinterpret_cast<Blue_Type*>(data);

				return (float)d[idx] / (float)d[256];
			},
			&rgb_scale_histogram.blues,
			255,
			0,
			nullptr,
			0,
			1.f,
			{ 0, 100 }
		);
	}
}

void Image::Grey_Scale_Histogram::compute(const sf::Texture& texture) noexcept {
	auto image_data = texture.copyToImage();
	auto pixels = image_data.getPixelsPtr();

	data[256] = 0;
	// >TODO see if i really need to do that.
	for (auto& x : data) x = 0;
	for (size_t i = 0; i < image_data.getSize().x * image_data.getSize().y * 4; i += 4) {
		auto sum = (size_t)((pixels[i + 0] + pixels[i + 1] + pixels[i + 2]) / 3.f);
		data[sum]++;
		data[256] = data[256] < data[sum] ? data[sum] : data[256];
	}
}

void Image::RGB_Histogram::compute(const sf::Texture& texture) noexcept {
	auto image_data = texture.copyToImage();
	auto pixels = image_data.getPixelsPtr();

	// >TODO see if i really need to do that.
	for (auto& x : reds) x = 0;
	for (auto& x : greens) x = 0;
	for (auto& x : blues) x = 0;

	for (size_t i = 0; i < image_data.getSize().x * image_data.getSize().y * 4; i += 4) {
		reds[pixels[i + 0]]++;
		greens[pixels[i + 1]]++;
		blues[pixels[i + 2]]++;

		reds[256]	= reds[256]		< reds[pixels[i + 0]]	? reds[pixels[i + 0]]	: reds[256];
		greens[256]	= greens[256]	< greens[pixels[i + 1]]	? greens[pixels[i + 1]]	: greens[256];
		blues[256]	= blues[256]	< blues[pixels[i + 2]]	? blues[pixels[i + 2]]	: blues[256];
	}
}

void Image::render(sf::RenderTarget& target) noexcept {
	auto last_view = target.getView();
	target.setView(target.getDefaultView());
	defer{ target.setView(last_view); };
	if (!open) return;

#ifndef NDEBUG

	sf::RectangleShape border;
	border.setPosition(get_global_position());
	border.setSize(get_size());
	border.setOutlineThickness(1);
	border.setOutlineColor(Vector4f{ 1, 0, 0, 1 });
	border.setFillColor(Vector4f{ 0, 0, 0, 0 });

	target.draw(border);
#endif

	if (taking_echantillon) {
		auto echantillon_pos = echantillon.pos + get_global_position();
		Vector2f echantillon_size = {
			get_size().x * (30.f / sprite.getTextureRect().width),
			get_size().y * (30.f / sprite.getTextureRect().height)
		};

		sf::RectangleShape echantillon_border;
		echantillon_border.setPosition(echantillon_pos);
		echantillon_border.setSize(echantillon_size);
		echantillon_border.setOutlineThickness(1);
		echantillon_border.setOutlineColor(Vector4f{ 1, 0, 0, 1 });
		echantillon_border.setFillColor(Vector4f{ 0, 0, 0, 0 });

		target.draw(echantillon_border);
	}
}

void Image::set_open(bool v) noexcept {
	open = v;
}
size_t Image::get_n() const noexcept {
	return n;
}

std::optional<Image::Echantillon_Data> Image::get_echantillon() const noexcept {
	if (!taking_echantillon) return {};
	Echantillon_Data d;
	d.pos.x = (size_t)(image.getSize().x * (echantillon.pos.x / get_size().x));
	d.pos.y = (size_t)(image.getSize().y * (echantillon.pos.y / get_size().y));
	d.pixels = &image;
	return d;
}

std::optional<sf::Sprite> Image::get_echantillon_sprite() const noexcept {
	if (!taking_echantillon) return {};

	sf::IntRect rec;
	rec.left = (int)((echantillon.pos.x / get_size().x) * sprite.getTextureRect().width);
	rec.top = (int)((echantillon.pos.y / get_size().y) * sprite.getTextureRect().height);
	rec.width = 30;
	rec.height = 30;

	sf::Sprite sample = sprite;
	sample.setTextureRect(rec);
	return sample;
}

bool Image::size_ok_for_sampling() const noexcept {
	return sprite.getTextureRect().width > 30 && sprite.getTextureRect().height > 30;
}
