#include "Image.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"

size_t Image::Total_N = 0;

Image::Image(sf::Texture& texture) noexcept {
	Total_N++;
	n = Total_N;
	sprite.setTexture(texture);
}

void Image::update(float dt) noexcept {
	Widget::update(dt);
	if (!is_visible()) return;
	if (!open) return;

	ImGui::Begin(std::to_string(n).c_str(), &open);
	set_global_position(ImGui::GetWindowPos());
	set_size(ImGui::GetWindowSize());
	auto window_size = ImGui::GetWindowSize();
	auto image_ratio = sprite.getTextureRect().width / sprite.getTextureRect().height;
	auto image_size = ((Vector2f)window_size).fitDownRatio(image_ratio);
	ImGui::Image(sprite, image_size);

	ImGui::Checkbox("Histogramme", &histogram_open);

	// I seriously CAN't WAIT for full compile time reflection in this language, ffs everyone else
	// already has it in one form or another...
	static const char* const histogram_type_list[] = { "Grey Scale", "RGB Scale" };
	ImGui::ListBox(
		"Histgoram type", (int*)&histogram_type, histogram_type_list, (int)Histogram_Type::Count
	);

	if (histogram_open) update_histogram();

	ImGui::End();
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
	auto image = texture.copyToImage();
	auto pixels = image.getPixelsPtr();

	data[256] = 0;
	// >TODO see if i really need to do that.
	for (auto& x : data) x = 0;
	for (size_t i = 0; i < image.getSize().x * image.getSize().y * 4; i += 4) {
		auto sum = (size_t)((pixels[i + 0] + pixels[i + 1] + pixels[i + 2]) / 3.f);
		data[sum]++;
		data[256] = data[256] < data[sum] ? data[sum] : data[256];
	}
}

void Image::RGB_Histogram::compute(const sf::Texture& texture) noexcept {
	auto image = texture.copyToImage();
	auto pixels = image.getPixelsPtr();

	// >TODO see if i really need to do that.
	for (auto& x : reds) x = 0;
	for (auto& x : greens) x = 0;
	for (auto& x : blues) x = 0;

	for (size_t i = 0; i < image.getSize().x * image.getSize().y * 4; i += 4) {
		reds[(size_t)pixels[i + 0]]++;
		greens[(size_t)pixels[i + 1]]++;
		blues[(size_t)pixels[i + 2]]++;

		reds[256]	= reds[256]		< reds[pixels[i + 0]]	? reds[pixels[i + 0]]	: reds[256];
		greens[256]	= greens[256]	< greens[pixels[i + 1]]	? greens[pixels[i + 1]]	: greens[256];
		blues[256]	= blues[256]	< blues[pixels[i + 2]]	? blues[pixels[i + 2]]	: blues[256];
	}
}

void Image::render(sf::RenderTarget& target) noexcept {
	auto last_view = target.getView();
	target.setView(target.getDefaultView());

#ifndef NDEBUG

	sf::RectangleShape border;
	border.setPosition(get_global_position());
	border.setSize(get_size());
	border.setOutlineThickness(1);
	border.setOutlineColor(Vector4f{ 1, 0, 0, 1 });
	border.setFillColor(Vector4f{ 0, 0, 0, 0 });

	target.draw(border);
#endif

	target.setView(last_view);
}

void Image::set_open(bool v) noexcept {
	open = v;
}
size_t Image::get_n() const noexcept {
	return n;
}
