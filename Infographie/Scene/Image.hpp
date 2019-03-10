#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <optional>
#include "Widget.hpp"

class Image : public Widget {
public:
	struct Echantillon_View {
		Vector2f pos{ 0, 0 };
	};

	// This is in pixels in contrast to the top one wich is in float pos (meaning that it's range
	// take into account the scale
	struct Echantillon_Data {
		Vector2u pos{ 0, 0 };

		const sf::Image* pixels{ nullptr };
		// ^-- carefull this stuff will live only as far a the image live.
	};

public:
	static size_t Total_N;

	Image(sf::Texture& texture) noexcept;

	void update(float dt) noexcept override;
	void update_histogram() noexcept;
	void update_echantillon() noexcept;

	void render(sf::RenderTarget& target) noexcept override;

	bool size_ok_for_sampling() const noexcept;

	void set_open(bool v) noexcept;
	bool is_open() const noexcept;

	size_t get_n() const noexcept;

	std::optional<sf::Sprite> get_echantillon_sprite() const noexcept;
	std::optional<Echantillon_Data> get_echantillon() const noexcept;

private:
	size_t n;
	bool open{ true };
	bool histogram_open{ false };

	enum Histogram_Type : int {
		Grey_Scale,
		RGB,
		Count
	} histogram_type{ Histogram_Type::Grey_Scale };
	struct Grey_Scale_Histogram {
		bool cached{ false };
		// 256 value and the last one is the max of the previous 256.
		std::array<size_t, 257> data{};

		void compute(const sf::Texture& texture) noexcept;
	} grey_scale_histogram;
	struct RGB_Histogram {
		bool cached{ false };
		// 256 value and the last one is the max of the previous 256.
		std::array<size_t, 257> reds{};
		std::array<size_t, 257> greens{};
		std::array<size_t, 257> blues{};

		void compute(const sf::Texture& texture) noexcept;
	} rgb_scale_histogram;

	bool taking_echantillon{ false };
	Echantillon_View echantillon;

	sf::Sprite sprite;
	sf::Image image;
};
