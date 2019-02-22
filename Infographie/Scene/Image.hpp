#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include "Widget.hpp"

class Image : public Widget {
public:
	static size_t Total_N;

	Image(sf::Texture& texture) noexcept;

	void update(float dt) noexcept override;
	void update_histogram() noexcept;

	void render(sf::RenderTarget& target) noexcept override;

	void set_open(bool v) noexcept;
	size_t get_n() const noexcept;

private:
	size_t n;
	bool open;
	bool histogram_open{ false };

	enum Histogram_Type : int {
		Grey_Scale,
		RGB,
		Count
	} histogram_type{ Histogram_Type::Grey_Scale };
	struct Grey_Scale_Histogram {
		bool cached{ false };
		// 256 value and the last one is the max of the previous 256.
		std::array<size_t, 257> data;

		void compute(const sf::Texture& texture) noexcept;
	} grey_scale_histogram;
	struct RGB_Histogram {
		bool cached{ false };
		// 256 value and the last one is the max of the previous 256.
		std::array<size_t, 257> reds;
		std::array<size_t, 257> greens;
		std::array<size_t, 257> blues;

		void compute(const sf::Texture& texture) noexcept;
	} rgb_scale_histogram;

	sf::Sprite sprite;
};
