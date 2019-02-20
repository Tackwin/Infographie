#pragma once
#include <bitset>
#include <memory>
#include <functional>
#include <type_traits>

#include <SFML/Graphics.hpp>

#include "Utils/UUID.hpp"
#include "Common.hpp"

#include "Math/Vector.hpp"
#include "Math/Rectangle.hpp"

class Widget {
public:
	struct Input_Mask { enum {
		Mouse_Began,
		Mouse_Going,
		Mouse_Ended,
		Hover_Began,
		Hover_Going,
		Hover_Ended,
		Key_Began,
		Key_Going,
		Key_Ended,
		Count
	}; };

	using ID_t = UUID;

	struct Callback {
		using type = std::function<bool(void)>;

		static const type FALSE;
		static const type TRUE;

		type began = FALSE;
		type ended = FALSE;
		type going = FALSE;
	};

	Widget() noexcept;
	Widget(const Widget& copy) = delete;
	Widget& operator=(const Widget&) = delete;
	Widget(Widget&&) = default;
	Widget& operator=(Widget&&) = default;
	Widget(Widget* const parent);
	virtual ~Widget();

	virtual Vector2f get_size() const;
	const Vector2f& get_origin() const;
	Vector2f get_global_position() const;
	const Vector2f& get_position() const;
	Rectangle2f get_global_bounding_box() const;
	bool is_visible() const;

	void set_on_focus(const Callback& on_focus) noexcept;
	void set_on_hover(const Callback& onHover);
	void set_on_click(const Callback& onClick);
	void set_on_key(const Callback& onKey);

	virtual void set_size(const Vector2f& size);
	void set_position(const Vector2f& pos);
	void set_global_position(const Vector2f& pos) noexcept;
	void set_origin(const Vector2f& origin);
	void set_origin_abs(const Vector2f& origin);
	void set_visible(bool visible);

	void emancipate();
	template<typename Child, typename... Args>
	std::enable_if_t<std::is_base_of_v<Widget, Child>, Child*> make_child(
		Args&&... args
	) noexcept {
		auto c = new Child{ std::forward<Args>(args)... };
		add_child(c);
		return c;
	}
	void deny_child(Widget* const child);
	void add_child(Widget* const child, int z = 0);
	bool have_child(const Widget* const child);
	bool have_child(UUID id) noexcept;
	void set_parent(Widget* const parent, int z = 0);
	Widget* const get_parent();
	const std::vector<std::unique_ptr<Widget>>& get_childs() const noexcept;
	Widget* find_child(UUID id) const noexcept;

	void kill_every_childs() noexcept;
	void kill_direct_child(UUID id) noexcept;

	virtual void update(float dt) noexcept;
	virtual void propagate_update(float dt) noexcept;

	virtual void render(sf::RenderTarget& target);
	virtual void propagate_render(sf::RenderTarget& target);

	std::bitset<9u> input(const std::bitset<9u>& mask);
	std::bitset<9u> propagate_input();
	std::bitset<9u> post_order_input(const std::bitset<9u>& mask);

	UUID get_uuid() const noexcept;

	virtual void set_focus(bool v) noexcept;
	virtual void lock_focus(bool v) noexcept;
	bool is_focus() const noexcept;

	int get_z_index() const noexcept;

protected: //god this is growing into a god class... :(
	int z_index = 0;
	Vector2f pos;
	Vector2f size;
	Vector2f origin;

	ID_t uuid;

	bool visible = true;
	bool pass_through = false;

	bool focused{ false };
	bool have_locked_focus{ false };
	bool hovered{ false };

	Widget* parent = nullptr;	//like, why do i even bother raw pointer mean, 
								//I DON'T HAVE THE OWNERSHIP, it settles it

	// TODO> investigate if this is the right thing to do. (having ownership on child)
	// i can't imagine a case where a child whose parent just got destroyed
	// still need to live. (also things like get_global_position would segfault)
	std::vector<std::unique_ptr<Widget>> childs;

	Callback on_hover;
	Callback on_click;
	Callback on_key;
	Callback on_focus;
};
