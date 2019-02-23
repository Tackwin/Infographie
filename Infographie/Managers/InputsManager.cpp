#include "InputsManager.hpp"

#include "Common.hpp"
#include "imgui/imgui-SFML.h"

sf::Uint32 InputsManager::textEntered;

bool InputsManager::wasKeyJustPressed = false;
bool InputsManager::wasKeyJustReleased = false;
int InputsManager::nKeyPressed = 0;

float InputsManager::lastScroll = 0.f;

bool InputsManager::keyPressed[sf::Keyboard::KeyCount];
bool InputsManager::keyJustPressed[sf::Keyboard::KeyCount];
bool InputsManager::keyJustReleased[sf::Keyboard::KeyCount];

bool InputsManager::mousePressed[sf::Mouse::ButtonCount];
bool InputsManager::mouseJustPressed[sf::Mouse::ButtonCount];
bool InputsManager::mouseJustReleased[sf::Mouse::ButtonCount];

Vector2f InputsManager::mouseScreenPos;
Vector2f InputsManager::mouseScreenDelta;

Vector2u InputsManager::windowsSize;

std::vector<sf::Keyboard::Key> InputsManager::currentSequence;

sf::Keyboard::Key InputsManager::lastKey;

void print_sequence(const std::vector<sf::Keyboard::Key>& x) {
	for (auto& k : x) {
		printf("%s ", IM::nameOfKey(k).c_str());
	}
	printf("\n");
}

InputsManager::InputsManager() {
}
InputsManager::~InputsManager() {
}

std::string InputsManager::nameOfKey(sf::Keyboard::Key k) noexcept {
#define X(a) case sf::Keyboard::a: return #a;
	switch (k)
	{
	X(Unknown)
	X(A)
	X(B)
	X(C)
	X(D)
	X(E)
	X(F)
	X(G)
	X(H)
	X(I)
	X(J)
	X(K)
	X(L)
	X(M)
	X(N)
	X(O)
	X(P)
	X(Q)
	X(R)
	X(S)
	X(T)
	X(U)
	X(V)
	X(W)
	X(X)
	X(Y)
	X(Z)
	X(Num0)
	X(Num1)
	X(Num2)
	X(Num3)
	X(Num4)
	X(Num5)
	X(Num6)
	X(Num7)
	X(Num8)
	X(Num9)
	X(Escape)
	X(LControl)
	X(LShift)
	X(LAlt)
	X(LSystem)
	X(RControl)
	X(RShift)
	X(RAlt)
	X(RSystem)
	X(Menu)
	X(LBracket)
	X(RBracket)
	X(SemiColon)
	X(Comma)
	X(Period)
	X(Quote)
	X(Slash)
	X(BackSlash)
	X(Tilde)
	X(Equal)
	X(Dash)
	X(Space)
	X(Return)
	X(BackSpace)
	X(Tab)
	X(PageUp)
	X(PageDown)
	X(End)
	X(Home)
	X(Insert)
	X(Delete)
	X(Add)
	X(Subtract)
	X(Multiply)
	X(Divide)
	X(Left)
	X(Right)
	X(Up)
	X(Down)
	X(Numpad0)
	X(Numpad1)
	X(Numpad2)
	X(Numpad3)
	X(Numpad4)
	X(Numpad5)
	X(Numpad6)
	X(Numpad7)
	X(Numpad8)
	X(Numpad9)
	X(F1)
	X(F2)
	X(F3)
	X(F4)
	X(F5)
	X(F6)
	X(F7)
	X(F8)
	X(F9)
	X(F10)
	X(F11)
	X(F12)
	X(F13)
	X(F14)
	X(F15)
	X(Pause)
	X(KeyCount)
	}
	return assert("should not happen !"), "";
#undef X
}

sf::Keyboard::Key InputsManager::getLastKeyPressed() noexcept {
	return lastKey;
}

bool InputsManager::isTextJustEntered() noexcept {
	return textEntered != 0;
}
sf::Uint32 InputsManager::getTextEntered() noexcept {
	return textEntered;
}

bool InputsManager::isLastSequence(
	std::initializer_list<sf::Keyboard::Key> keys,
	std::initializer_list<sf::Keyboard::Key> modifiers
) noexcept {
	if (keys.size() > currentSequence.size()) return false;

	auto end_pair = std::pair{ std::end(keys), std::end(currentSequence) };
	for (
		auto& [i, j] = end_pair;
		i != std::begin(keys);
		--i, --j
	) {
		if (*(i - 1) != *(j - 1)) return false;
	}

	for (auto& x : modifiers) if (!isKeyPressed(x)) return false;

	return true;
}
bool InputsManager::isLastSequenceJustFinished(
	std::initializer_list<sf::Keyboard::Key> keys,
	std::initializer_list<sf::Keyboard::Key> modifiers
) noexcept {
	return (wasKeyJustPressed && isLastSequence(keys, modifiers));
}

bool InputsManager::isKeyJustPressed() noexcept {
	return wasKeyJustPressed;
}
bool InputsManager::isKeyJustReleased() noexcept {
	return wasKeyJustReleased;
}

int InputsManager::countKeyPressed() noexcept {
	return nKeyPressed;
}

bool InputsManager::isKeyPressed() noexcept {
	return nKeyPressed != 0;
}
bool InputsManager::isKeyPressed(const sf::Keyboard::Key &key) {
	return keyPressed[key];
}
bool InputsManager::isKeyJustPressed(const sf::Keyboard::Key &key) {
	return keyJustPressed[key];
}

bool InputsManager::isKeyJustReleased(const sf::Keyboard::Key &key) {
	return keyJustReleased[key];
}

bool InputsManager::isMousePressed(const sf::Mouse::Button &button) {
	return mousePressed[button];
}
bool IM::is_one_of_pressed(const std::vector<sf::Mouse::Button>& buttons) noexcept {
	for (auto& m : buttons) {
		if (isMousePressed(m)) return true;
	}
	return false;
}
bool IM::is_one_of_just_released(const std::vector<sf::Mouse::Button>& buttons) noexcept {
	for (auto& m : buttons) {
		if (isMouseJustReleased(m)) return true;
	}
	return false;
}
bool IM::is_one_of_just_pressed(const std::vector<sf::Mouse::Button>& buttons) noexcept {
	for (auto& m : buttons) {
		if (isMouseJustPressed(m)) return true;
	}
	return false;
}

bool InputsManager::isMouseJustPressed(const sf::Mouse::Button &button) {
	return mouseJustPressed[button];
}
bool InputsManager::isMouseJustReleased(const sf::Mouse::Button &button) {
	return mouseJustReleased[button];
}

Vector2f InputsManager::getMousePosInView(const sf::View& view) {
	return applyInverseView(view, getMouseScreenPos());
}
Vector2f InputsManager::getMouseDeltaInView(const sf::View& view) noexcept {
	auto A = getMouseScreenPos();
	auto B = A - getMouseScreenDelta();

	return applyInverseView(view, A) - applyInverseView(view, B);
}
Vector2f InputsManager::getMouseScreenPos() {
	return mouseScreenPos;
}
Vector2f InputsManager::getMouseScreenDelta() noexcept {
	return mouseScreenDelta;
}
float InputsManager::getLastScroll() noexcept {
	return lastScroll;
}

void InputsManager::update(sf::RenderWindow &window) {
	wasKeyJustPressed = false;
	wasKeyJustReleased = false;
	nKeyPressed = std::max(nKeyPressed, 0);
	
	textEntered = 0;
	lastScroll = 0;

	memset(keyJustPressed   , 0, sizeof(keyJustPressed));
	memset(keyJustReleased  , 0, sizeof(keyJustReleased));
	memset(mouseJustPressed , 0, sizeof(mouseJustPressed));
	memset(mouseJustReleased, 0, sizeof(mouseJustReleased));

	for (size_t i = 0; i < sf::Keyboard::KeyCount; ++i) {
		auto pressed = sf::Keyboard::isKeyPressed((sf::Keyboard::Key)i);

		keyJustPressed[i] = !keyPressed[i] && pressed;
		wasKeyJustPressed |= keyJustPressed[i];
		keyJustReleased[i] = keyPressed[i] && !pressed;
		wasKeyJustReleased |= keyJustReleased[i];
		keyPressed[i] = pressed;

		if (keyJustPressed[i]) {
			if (currentSequence.size() == MAX_SEQUENCE) {
				std::rotate(
					std::begin(currentSequence),
					std::begin(currentSequence) + 1,
					std::end(currentSequence)
				);
				currentSequence.back() = (sf::Keyboard::Key)i;
			}
			else {
				currentSequence.push_back((sf::Keyboard::Key)i);
			}

		}
	}

	for (size_t i = 0; i < sf::Mouse::ButtonCount; ++i) {
		auto pressed = sf::Mouse::isButtonPressed((sf::Mouse::Button)i);

		mouseJustPressed[i] = !mousePressed[i] && pressed;
		mouseJustReleased[i] = mousePressed[i] && !pressed;
		mousePressed[i] = pressed;
	}

	sf::Event event;
	while(window.pollEvent(event)) {
		ImGui::SFML::ProcessEvent(event);

		if(event.type == sf::Event::Closed)
			window.close();

		if(event.type == sf::Event::KeyPressed) {
			nKeyPressed++;
			if (event.key.code == sf::Keyboard::Unknown)
				continue;

			lastKey = event.key.code;
		}
		if(event.type == sf::Event::KeyReleased) {
			nKeyPressed--;
			if (event.key.code == sf::Keyboard::Unknown)
				continue;
		}
		if (event.type == sf::Event::TextEntered) {
			textEntered = event.text.unicode;
		}
		if (event.type == sf::Event::MouseWheelScrolled) {
			lastScroll = event.mouseWheelScroll.delta;
		}
	}

	mouseScreenDelta = (Vector2f)sf::Mouse::getPosition(window) - mouseScreenPos;
	mouseScreenPos = sf::Mouse::getPosition(window);

	windowsSize = window.getSize();
}


Vector2f InputsManager::applyInverseView(const sf::View& view, Vector2f p) noexcept {
	const auto& viewScope = view.getViewport();
	auto viewPort = sf::IntRect(
		(int)std::ceil(windowsSize.x * viewScope.left),
		(int)std::ceil(windowsSize.y * viewScope.top),
		(int)std::ceil(windowsSize.x * viewScope.width),
		(int)std::ceil(windowsSize.y * viewScope.height)
	);

	Vector2f normalized;

	normalized.x = -1.f + 2.f * (p.x - viewPort.left) / viewPort.width;
	normalized.y = +1.f - 2.f * (p.y - viewPort.top) / viewPort.height;

	return view.getInverseTransform().transformPoint(normalized);
}