#include "Windows/Mouse.h"
#include "Windows/framework.h"

std::optional<Mouse::Event> Mouse::Read() noexcept {
	if (buffer.size() > 0) {
		Mouse::Event e = buffer.front();
		buffer.pop();
		return e;
	}
	return {};
}

void Mouse::Flush() noexcept {
	buffer = std::queue<Event>();
}

void Mouse::OnMouseMove(i32 x_, i32 y_) noexcept {
	x = x_;
	y = y_;

	buffer.push(Event(Event::Type::Move, *this));
	TrimBuffer();
}

void Mouse::OnMouseLeave() noexcept {
	inWindow = false;

	buffer.push(Event(Event::Type::Leave, *this));
	TrimBuffer();
}

void Mouse::OnMouseEnter() noexcept {
	inWindow = true;

	buffer.push(Event(Event::Type::Enter, *this));
	TrimBuffer();
}

#pragma warning(push)
#pragma warning(disable: 4100)
void Mouse::OnLeftPressed(i32 _x, i32 _y) noexcept {
	leftPressed = true;

	buffer.push(Event(Event::Type::LPress, *this));
	TrimBuffer();
}

void Mouse::OnLeftReleased(i32 _x, i32 _y) noexcept {
	leftPressed = false;

	buffer.push(Event(Event::Type::LRelease, *this));
	TrimBuffer();
}

void Mouse::OnRightPressed(i32 _x, i32 _y) noexcept {
	rightPressed = true;

	buffer.push(Event(Event::Type::RPress, *this));
	TrimBuffer();
}

void Mouse::OnRightReleased(i32 _x, i32 _y) noexcept {
	rightPressed = false;

	buffer.push(Event(Event::Type::RRelease, *this));
	TrimBuffer();
}

void Mouse::OnWheelUp(i32 _x, i32 _y) noexcept {
	buffer.push(Event(Event::Type::WheelUp, *this));
	TrimBuffer();
}

void Mouse::OnWheelDown(i32 _x, i32 _y) noexcept {
	buffer.push(Event(Event::Type::WheelDown, *this));
	TrimBuffer();
}
#pragma warning(pop)

void Mouse::OnWheelDelta(i32 x_, i32 y_, i32 delta) noexcept {
	wheelDeltaCarry += delta;
	while (wheelDeltaCarry >= WHEEL_DELTA) {
		wheelDeltaCarry -= WHEEL_DELTA;
		OnWheelUp(x_, y_);
	}
	while (wheelDeltaCarry <= WHEEL_DELTA) {
		wheelDeltaCarry += WHEEL_DELTA;
		OnWheelDown(x_, y_);
	}
}

void Mouse::TrimBuffer() noexcept {
	while (buffer.size() > bufferSize) buffer.pop();
}
