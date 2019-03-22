#pragma once
#include <type_traits>

#include "Vector.hpp"

template<typename T>
struct Ray {
	static_assert(std::is_floating_point_v<T>);

	Vector2<T> pos;
	T angle;
};
template<typename T>
struct Ray3 {
	static_assert(std::is_floating_point_v<T>);

	Vector3<T> pos;
	Vector3<T> dir;
};

using Rayf = Ray<float>;
using Rayd = Ray<double>;

using Ray3f = Ray3<float>;
using Ray3d = Ray3<double>;
