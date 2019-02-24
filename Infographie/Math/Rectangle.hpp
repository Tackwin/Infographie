#pragma once
#include "Vector.hpp"

template<typename T>
struct Rectangle_t {
	union {
		struct {
			Vector<2, T> pos;
			Vector<2, T> size;
		};
		struct {
			T x;
			T y;
			T w;
			T h;
		};
	};

	Rectangle_t() {}

	Rectangle_t(const Vector<2, T>& pos, const Vector<2, T>& size) :
		pos(pos),
		size(size) 
	{}
#ifdef SFML_GRAPHICS_HPP

	Rectangle_t(const sf::FloatRect& rec) : 
		x(rec.left), y(rec.top), w(rec.width), h(rec.height)
	{}

#endif

	bool intersect(const Rectangle_t<T>& other) const {
		return !(
				pos.x + size.x < other.pos.x || pos.x > other.pos.x + other.size.x ||
				pos.y + size.y < other.pos.y || pos.y > other.pos.y + other.size.y
			);
	}

	// works only for top down y
	bool isOnTopOf(Rectangle_t<T> other) const {
		T distEdgeToEdge = std::max(other.x + other.w - x, x + w - other.x);
		T sumOfWidth = w + other.w;

		return y < other.y && distEdgeToEdge < sumOfWidth;
	}
	bool isFullyOnTopOf(Rectangle_t<T> other, T tolerance = FLT_EPSILON) const noexcept {
		return y + h < other.y + tolerance;
	}

	// works only for top down y
	bool isOnBotOf(Rectangle_t<T> other) const {
		T distEdgeToEdge = std::max(other.x + other.w - x, x + w - other.x);
		T sumOfWidth = w + other.w;

		return y > other.y && distEdgeToEdge < sumOfWidth;
	}

	Vector<2, T> center() const {
		return pos + size / 2;
	}

	void setCenter(Vector2<T> vec) noexcept {
		pos = vec - size / 2.0;
	}

	T bot() const {
		return pos.y + size.y;
	}

	std::tuple<
		Rectangle_t<T>, Rectangle_t<T>, Rectangle_t<T>, Rectangle_t<T>
	> divide() const noexcept {
		return {
			Rectangle_t<T>{pos, size / 2},
			Rectangle_t<T>{ {pos.x + size.x / 2, pos.y}, size / 2 },
			Rectangle_t<T>{ {pos.x, pos.y + size.y / 2}, size / 2 },
			Rectangle_t<T>{pos + size / 2, size / 2},
		};
	}

	// >TODO: implement this, for now we treat this as if it were the support
	// of a circle of radius max(w, h) / 2
	Vector2<T> support(T a, T d) const noexcept {
		return Vector2<T>::createUnitVector((double)a) * (d + std::max({ w, h }) / 2);
	}

	Vector2<T> topLeft() const noexcept {
		return pos;
	}
	Vector2<T> topRight() const noexcept {
		return { x + w, y };
	}
	Vector2<T> botLeft() const noexcept {
		return { x, y + h };
	}
	Vector2<T> botRight() const noexcept {
		return pos + size;
	}

	Rectangle_t<T> fitUpRatio(double ratio) const noexcept {
		if (w > h) {
			return { pos,{ w, (T)(w / ratio) } };
		}
		else {
			return { pos,{ (T)(h * ratio), h } };
		}
	}
	Rectangle_t<T> fitDownRatio(double ratio) const noexcept {
		if (w < h) {
			return { pos,{ w, (T)(w / ratio) } };
		}
		else {
			return { pos,{ (T)(h * ratio), h } };
		}
	}

	Rectangle_t<T> restrictIn(Rectangle_t<T> area) const noexcept {
		Rectangle_t<T> result = *this;

		if (w > area.w) {
			result.x = area.center().x - result.size.x / 2;
		}
		else {
			// there is a reason it's made like that
			// if for instance area is a narrow (in height) space, then *this can jump up
			// and down if instead of if (result.x ...) it were if (x ...)
			if (x + w > area.x + area.w) {
				result.x = area.x + area.w - w;
			}
			if (result.x < area.x) {
				result.x = area.x;
			}
		}

		if (h > area.h) {
			result.y = area.center().y - result.size.y / 2;
		} else {
			if (y + h > area.y + area.h) {
				result.y = area.y + area.h - h;
			}
			if (result.y < area.y) {
				result.y = area.y;
			}
		}
		return result;
	}

	T area() const noexcept {
		return w * h;
	}

	template<typename U>
	bool in(const Vector<2, U>& p) const {
		return p.inRect(pos, size);
	}

#ifdef SFML_GRAPHICS_HPP

	void render(sf::RenderTarget& target, Vector4f color) const noexcept {
		sf::RectangleShape shape{ size };
		shape.setPosition(pos);
		shape.setFillColor(color);
		target.draw(shape);
	}
	void render(
		sf::RenderTarget& target, Vector4f in, Vector4f on, float thick = 0.01f
	) const noexcept {
		sf::RectangleShape shape{ size };
		shape.setPosition(pos);
		shape.setFillColor(in);
		shape.setOutlineColor(on);
		shape.setOutlineThickness(thick * std::min(w, h));
		target.draw(shape);
	}

#endif

	static Rectangle_t<T> hull(std::vector<Rectangle_t<T>> recs) noexcept {
		Rectangle_t<T> hull = recs[0];

		for (auto rec : recs) {
			hull.x = std::min(rec.x, hull.x);
			hull.y = std::min(rec.y, hull.y);
		}
		for (auto rec : recs) {
			hull.w = std::max(rec.x + rec.w, hull.w + hull.x) - hull.x;
			hull.h = std::max(rec.y + rec.h, hull.h + hull.y) - hull.y;
		}
		return hull;
	}

};

template<typename T>
using Rectangle2 = Rectangle_t<T>;
using Rectangle2f = Rectangle2<float>;
using Rectangle2d = Rectangle2<double>;
/*

// >SEE
// This stuff is usefull for my game Boss Room, i keep it here just in case i end up pulling over
// my json class serializer (dyn_struct)

template<typename T>
void from_dyn_struct(const dyn_struct& d_struct, Rectangle<T>& x) noexcept {
	assert(holds_array(d_struct));

	if (size(d_struct) == 2) {
		x.pos = (Vector2<T>)d_struct[0];
		x.size = (Vector2<T>)d_struct[1];
	}
	else {
		x.x = (T)d_struct[0];
		x.y = (T)d_struct[1];
		x.w = (T)d_struct[2];
		x.h = (T)d_struct[3];
	}
}

template<typename T>
void to_dyn_struct(dyn_struct& d_struct, const Rectangle<T>& x) noexcept {
	d_struct = dyn_struct_array(4);
	d_struct[0] = x.x;
	d_struct[1] = x.y;
	d_struct[2] = x.w;
	d_struct[3] = x.h;
}
*/

