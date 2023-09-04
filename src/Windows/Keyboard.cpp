#include "Windows/Keyboard.h"

void Keyboard::Update() noexcept {
	keyStatesPrev = keyStates;
}

bool Keyboard::KeyIsPressed(u8 code) const noexcept { return keyStates[code]; }

bool Keyboard::KeyPressed(u8 code) const noexcept {
	return !keyStatesPrev[code] && keyStates[code];
}

bool Keyboard::KeyReleased(u8 code) const noexcept {
	return keyStatesPrev[code] && !keyStates[code];
}

Keyboard::Event Keyboard::ReadKey() noexcept {
	if (keyBuffer.size() > 0) {
		Keyboard::Event e = keyBuffer.front();
		keyBuffer.pop();
		return e;
	} else return Keyboard::Event();
}

bool Keyboard::KeyIsEmpty() const noexcept { return keyBuffer.empty(); }

char Keyboard::ReadChar() noexcept {
	if (charBuffer.size() > 0) {
		u8 _char = charBuffer.front();
		charBuffer.pop();
		return _char;
	} else return 0;
}

bool Keyboard::CharIsEmpty() const noexcept { return charBuffer.empty(); }

void Keyboard::ClearKey() noexcept { keyBuffer = std::queue<Event>(); }
void Keyboard::ClearChar() noexcept { charBuffer = std::queue<char>(); }
void Keyboard::Clear() noexcept { ClearKey(); ClearChar(); }

void Keyboard::EnableAutorepeat() noexcept { autorepeatEnabled = true; }
void Keyboard::DisableAutorepeat() noexcept { autorepeatEnabled = false; }
bool Keyboard::AutorepeatIsEnabled() const noexcept { return autorepeatEnabled; }

void Keyboard::OnKeyPressed(u8 code) noexcept {
	keyStates[code] = true;
	keyBuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, code));
	TrimBuffer(keyBuffer);
}

void Keyboard::OnKeyReleased(u8 code) noexcept {
	keyStates[code] = false;
	keyBuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, code));
	TrimBuffer(keyBuffer);
}

void Keyboard::OnChar(char _char) noexcept {
	charBuffer.push(_char);
	TrimBuffer(keyBuffer);
}

void Keyboard::ClearState() noexcept { keyStates.reset(); }