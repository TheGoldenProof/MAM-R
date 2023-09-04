#pragma once
#include "TGLib/TGLib.h"
#include <queue>
#include <optional>

class Mouse {
	friend class Window;
public:
	class Event {
	public: 
		enum class Type {
			LPress, LRelease,
			RPress, RRelease,
			WheelUp, WheelDown,
			Move, Enter, Leave,
		};
	public:
		Event(Type type, Mouse const& parent) noexcept :
			type(type), 
			leftPressed(parent.leftPressed), rightPressed(parent.rightPressed), 
			x(parent.x), y(parent.y) {}
		
		Type GetType() const noexcept { return type; }
		std::pair<i32, i32> GetPos() const noexcept { return {x, y}; }
		i32 GetPosX() const noexcept { return x; }
		i32 GetPosY() const noexcept { return y; }
		bool IsLeftPressed() const noexcept { return leftPressed; }
		bool IsRightPressed() const noexcept { return rightPressed; }
	private:
		Type type;
		bool leftPressed, rightPressed;
		i32 x, y;
	};
public:
	Mouse() = default;
	Mouse(Mouse const&) = delete;
	Mouse& operator=(Mouse const&) = delete;

	std::pair<i32, i32> GetPos() const noexcept { return {x, y}; }
	i32 GetPosX() const noexcept { return x; }
	i32 GetPosY() const noexcept { return y; }
	bool IsLeftPressed() const noexcept { return leftPressed; }
	bool IsRightPressed() const noexcept { return rightPressed; }
	bool IsInWindow() const noexcept { return inWindow; }
	bool IsEmpty() const noexcept { return buffer.empty(); }

	std::optional<Mouse::Event> Read() noexcept;
	void Flush() noexcept;
private:
	void OnMouseMove(i32 x, i32 y) noexcept;
	void OnMouseLeave() noexcept;
	void OnMouseEnter() noexcept;
	void OnLeftPressed(i32 x, i32 y) noexcept;
	void OnLeftReleased(i32 x, i32 y) noexcept;
	void OnRightPressed(i32 x, i32 y) noexcept;
	void OnRightReleased(i32 x, i32 y) noexcept;
	void OnWheelUp(i32 x, i32 y) noexcept;
	void OnWheelDown(i32 x, i32 y) noexcept;
	void OnWheelDelta(i32 x, i32 y, i32 delta) noexcept;
	
	void TrimBuffer() noexcept;
private:
	static constexpr u32 bufferSize = 16;
	i32 x = 0, y = 0;
	bool leftPressed = false, rightPressed = false;
	bool inWindow = false;
	i32 wheelDeltaCarry = 0;
	std::queue<Event> buffer;
};