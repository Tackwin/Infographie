#include "Widget.hpp"

#include <queue>
#include <stack>

#include "Managers/InputsManager.hpp"

const Widget::Callback::type Widget::Callback::False_V = []() { return false; };
const Widget::Callback::type Widget::Callback::True_V =  []() { return true; };

Widget::Widget() noexcept {};
Widget::Widget(Widget* const parent) : parent(parent) {}
Widget::~Widget() {}

void Widget::emancipate(){
	if (!parent)
		return;

	if (parent->have_child(this))
		parent->deny_child(this);

	parent = nullptr;
}
void Widget::deny_child(Widget* const child) {
	if (!have_child(child))
		return;

	const auto& it = std::find_if(childs.begin(), childs.end(), 
		[child](const std::unique_ptr<Widget>& A) -> bool {
			return A.get() == child;
		}
	);

	Widget* c = it->get();
	childs.erase(it);
	c->emancipate();
}
void Widget::add_child(Widget* const child, int z) {
	if (!child) return;

	if (child == parent)
		return;

	if (!have_child(child)) {
		child->z_index = z;
		childs.push_back(std::unique_ptr<Widget>{ child });
		std::sort(childs.begin(), childs.end(), 
			[](	const std::unique_ptr<Widget>& A, const std::unique_ptr<Widget>& B) -> bool {
				return A->get_z_index() < B->get_z_index();
			}
		);
	}
	if (child->get_parent() != this) {
		child->set_parent(this);
	}
}
void Widget::kill_every_childs() noexcept {
	childs.clear();
}
void Widget::kill_direct_child(Uuid_t id) noexcept {
	auto it = std::find_if(std::begin(childs), std::end(childs), [id](const auto& x) {
		return x->get_uuid() == id;
	});

	if (it != std::end(childs)) {
		childs.erase(it);
	}
}

bool Widget::have_child(const Widget* const child) {
	if (!child) return false;

	const auto& it = std::find_if(childs.begin(), childs.end(),
		[child](const auto& A) -> bool {
		return A.get() == child;
	});

	return it != childs.end();
}
bool Widget::have_child(Uuid_t id) noexcept {
	return std::find_if(std::begin(childs), std::end(childs), [id](const auto& x) {
		return x->get_uuid() == id;
	}) != std::end(childs);
}
void Widget::set_parent(Widget* const p, int z) {
	assert(p);

	if (have_child(p))
		return;

	if (parent) {
		parent->deny_child(this);
	}

	parent = p;
	if (!p->have_child(this)) {
		p->add_child(this, z);
	}
}
Widget* const Widget::get_parent() {
	return parent;
}


Vector2f Widget::get_size() const {
	return size;
}
const Vector2f& Widget::get_origin() const {
	return origin;
}
const Vector2f& Widget::get_position() const {
	return pos;
}
Vector2f Widget::get_global_position() const {
	return pos + (parent ? parent->get_global_position() : Vector2f{0, 0});
}
Rectangle2f Widget::get_global_bounding_box() const {
	return {
		get_global_position() - Vector2f(origin.x * size.x, origin.y * size.y),
		get_size()
	};
}
bool Widget::is_visible() const {
	return visible;
}


void Widget::set_size(const Vector2f& s) {
	size = s;
}
void Widget::set_position(const Vector2f& p) {
	pos = p;
}
void Widget::set_global_position(const Vector2f& p) noexcept {
	pos = p - (parent ? parent->get_global_position() : Vector2f{ 0, 0 });
}
void Widget::set_origin(const Vector2f& o) {
	origin = o;
}
void Widget::set_origin_abs(const Vector2f& o) {
	origin.x = o.x / get_size().x;
	origin.y = o.y / get_size().y;
}

void Widget3::set_size(Vector3f s) noexcept {
	size3 = s;
}
void Widget3::set_position(Vector3f p) noexcept {
	pos3 = p;
}
void Widget3::set_global_position(Vector3f p) noexcept {
	if (auto parent3 = dynamic_cast<Widget3*>(parent)) {
		pos3 = p - (parent3 ? parent3->get_global_position3() : Vector3f{ 0, 0, 0 });
	}
	else {
		pos3 = p - Vector3f{
			(parent ? parent->get_global_position() : Vector2f{ 0, 0 }).x,
			(parent ? parent->get_global_position() : Vector2f{ 0, 0 }).y,
			0
		};
	}
}
void Widget3::set_origin(Vector3f o) noexcept {
	origin3 = o;
}
void Widget3::set_origin_abs(Vector3f o) noexcept {
	origin3.x = o.x / get_size3().x;
	origin3.y = o.y / get_size3().y;
	origin3.z = o.z / get_size3().z;
}


Vector3f Widget3::get_size3() const noexcept {
	return size3;
}
Vector3f Widget3::get_origin3() const noexcept {
	return origin3;
}
Vector3f Widget3::get_global_position3() const noexcept {
	if (auto parent3 = dynamic_cast<Widget3*>(parent)) {
		return pos3 + (parent3 ? parent3->get_global_position3() : Vector3f{ 0, 0, 0 });
	}
	else {
		return pos3 + Vector3f{
			(parent ? parent->get_global_position() : Vector2f{ 0, 0 }).x,
			(parent ? parent->get_global_position() : Vector2f{ 0, 0 }).y,
			0
		};
	}
}
Vector3f Widget3::get_position3() const noexcept {
	return pos3;
}

void Widget::set_visible(bool v) {
	visible = v;
}

const std::vector<std::unique_ptr<Widget>>& Widget::get_childs() const noexcept {
	return childs;
}

Widget* Widget::find_parent(const type_info& type) noexcept {
	auto next_up = parent;

	while (next_up) {
		if (typeid(*next_up) == type) return next_up;
		next_up = next_up->parent;
	}

	return next_up;
}

Widget* Widget::find_child(Uuid_t id) const noexcept {

	std::queue<Widget*> open;
	for (auto& child : get_childs()) {
		if (child->get_uuid() == id) return child.get();
		open.push(child.get());
	}

	while (!open.empty()) {
		auto w = open.front();
		const auto& c = w->get_childs();
		open.pop();

		auto it = std::find_if(std::begin(c), std::end(c),
			[id](const auto& a) -> bool {
				return a->get_uuid() == id;
			}
		);
		if (it != std::end(c)) {
			return it->get();
		}

		for (auto& child : c) {
			open.push(child.get());
		}
	}

	return nullptr;
}

void Widget::render(sf::RenderTarget&) {
	if (!visible) return;

	// sf::CircleShape mark{ 2.f };
	// mark.setOrigin(mark.getRadius(), mark.getRadius());
	// mark.setPosition(get_global_position());
	// mark.setFillColor(Vector4d{ 1, 1, 1, 0.2 });
	// target.draw(mark);
}
void Widget::propagate_render(sf::RenderTarget& target) {
	if (!is_visible()) return;
	render(target);

	for (const auto& c : childs) {
		c->propagate_render(target);
	}
}

void Widget::update(float) noexcept {
	if (!is_visible()) return;
}
void Widget::propagate_update(float dt) noexcept {
	if (!is_visible()) return;
	update(dt);

	for (const auto& c : childs) {
		c->propagate_update(dt);
	}
}

std::bitset<9u> Widget::input(const std::bitset<9u>& mask) {
	std::bitset<9u> result;
	result.reset();

	auto mouseIsIn = get_global_bounding_box().in(IM::getMouseScreenPos());

	if (mouseIsIn || focused) {
		std::vector<sf::Mouse::Button> buttons = {
			sf::Mouse::Left, sf::Mouse::Right, sf::Mouse::Middle
		};

		if (IM::is_one_of_just_pressed(buttons) && !mask[0]) {
			if (!mouseIsIn && !have_locked_focus)	set_focus(false);
			else							result[0] = on_click.began();
		}
		if (IM::is_one_of_pressed(buttons) && !mask[1]) {
			result[1] = on_click.going();
		}
		if (IM::is_one_of_just_released(buttons) && !mask[2]) {
			result[2] = on_click.ended();
		}
		if (!hovered) {
			if (!mask[3]) {
				result[3] = on_hover.began();
				hovered = true;
			}
		}
		else {
			if (!mask[4]) {
				result[4] = on_hover.going();
			}
		}
	}
	else if (!mouseIsIn) {
		if (hovered) {
			hovered = false;
			if (!mask[5]) {
				result[5] = on_hover.ended();
			}
		}
	}

	if (focused) {
		on_focus.going();

		if (IM::isKeyJustPressed() && !mask[6]) {
			result[6] = on_key.began();
		} if (IM::isKeyPressed() && !mask[7]) {
			result[7] = on_key.going();
		} if (IM::isKeyJustReleased() && !mask[8]) {
			result[8] = on_key.ended();
		}
	}

	return result;
}

std::bitset<9u> Widget::propagate_input() {
	return post_order_input({});
}

std::bitset<9u> Widget::post_order_input(const std::bitset<9>& mask) {
	if (mask.all()) 
		return mask;

	auto maskAfterChild = mask;
	for (auto& c : get_childs()) {

		if (!c->is_visible()) 
			continue;

		maskAfterChild |= c->post_order_input(mask);
	}

	maskAfterChild |= input(maskAfterChild);
	return maskAfterChild;
}


/*
								*
							   / \
							  *   *
							 /|\  |\
							* * * * *
						   /|   |    
						  * *   *    
*/

void Widget::set_on_focus(const Callback& c) noexcept {
	this->on_focus = c;
}
void Widget::set_on_hover(const Callback& c) {
	on_hover = c;
}
void Widget::set_on_click(const Callback& c) {
	on_click = c;
}
void Widget::set_on_key(const Callback& c) {
	on_key = c;
}

Uuid_t Widget::get_uuid() const noexcept {
	return uuid;
}

void Widget::set_focus(bool v) noexcept {
	if (v != focused) { if (v) on_focus.began(); else on_focus.ended(); }

	focused = v;
}
void Widget::lock_focus(bool v) noexcept {
	have_locked_focus = v;
}
bool Widget::is_focus() const noexcept {
	return focused;
}

int Widget::get_z_index() const noexcept {
	return z_index;
}

// by default every widget is transparent to mouse picking
std::optional<Vector3f> Widget3::is_selected(Vector3f, Vector3f) const noexcept {
	return std::nullopt;
}

void Widget::for_every_childs(std::function<void(Widget*)>&& f) noexcept {
	std::vector<Widget*> list;
	for (auto& x : childs) list.push_back(x.get());

	while (!list.empty()) {
		auto x = list.back();
		list.pop_back();

		f(x);

		for (auto& y : x->get_childs()) {
			list.push_back(y.get());
		}
	}
}


void Widget3::opengl_render() noexcept {}
void Widget3::last_opengl_render() noexcept {}

void Widget3::propagate_opengl_render() noexcept {
	size_t i = 0;

	for (; i < childs.size(); ++i) {
		if (childs[i]->get_z_index() >= 0) break;
		if (auto c3 = dynamic_cast<Widget3*>(childs[i].get())) c3->propagate_opengl_render();
	}

	if (!is_visible()) return;

	opengl_render();

	for (; i < childs.size(); ++i) {
		if (auto c3 = dynamic_cast<Widget3*>(childs[i].get())) c3->propagate_opengl_render();
	}
}

void Widget3::propagate_last_opengl_render() noexcept {
	size_t i = 0;

	for (; i < childs.size(); ++i) {
		if (childs[i]->get_z_index() >= 0) break;
		if (auto c3 = dynamic_cast<Widget3*>(childs[i].get())) c3->propagate_last_opengl_render();
	}

	if (!is_visible()) return;

	last_opengl_render();

	for (; i < childs.size(); ++i) {
		if (auto c3 = dynamic_cast<Widget3*>(childs[i].get())) c3->propagate_last_opengl_render();
	}
}

void Widget3::set_rotation(Vector3f r) noexcept {
	rotation3 = r;
}
Vector3f Widget3::get_rotation3() const noexcept {
	return rotation3;
}

void Widget::set_name(std::string x) noexcept {
	name = std::move(x);
}
const std::string& Widget::get_name() const noexcept {
	return name;
}

