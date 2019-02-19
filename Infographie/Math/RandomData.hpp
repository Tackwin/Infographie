#pragma once
#include <random>
#include <optional>
#include <type_traits>

#include "3rd/json.hpp"

#include "Common.hpp"

template<typename T>
struct RandomData {

	enum DistributionKind {
		Normal,
		Uniform,
		Count
	};

	T mean;
	T range;
	std::optional<T> min{};
	std::optional<T> max{};
	DistributionKind kind{ Count };

	T operator()() const noexcept {
		switch (kind)
		{
		case RandomData<float>::Normal: {
			std::normal_distribution<T> dist{ mean, range };
			auto x = dist(RD);

			if (min && x < *min) x = *min;
			if (max && *max < x) x = *max;

			return x;
		}
		case RandomData<float>::Uniform: {
			if constexpr (std::is_floating_point_v<T>) {
				std::uniform_real_distribution<T> dist{ mean, range };
				auto x = dist(RD);

				if (min && x < *min) x = *min;
				if (max && *max < x) x = *max;

				return x;
			}
			else {
				std::uniform_int_distribution<T> dist{ mean, range };
				auto x = dist(RD);

				if (min && x < *min) x = *min;
				if (max && *max < x) x = *max;

				return x;
			}
		}
		}
		assert(":(");
		return {};
	}
};

template<typename T>
void to_json(nlohmann::json& j, const RandomData<T>& d) noexcept {
	j = {
		{"range", d.range},
		{"mean", d.mean},
		{"kind", (int)d.kind}
	};
	if (d.max) j["max"] = *d.max;
	if (d.min) j["min"] = *d.min;
}

template<typename T>
void from_json(const nlohmann::json& j, RandomData<T>& d) noexcept {
	d.kind = j.at("kind").get<RandomData<T>::DistributionKind>();
	d.range = j.at("range");
	d.mean = j.at("mean");
	if (j.count("min") > 0) d.min = j.at("min");
	if (j.count("max") > 0) d.max = j.at("max");
}