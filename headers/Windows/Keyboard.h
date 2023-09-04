#pragma once

#include <queue>
#include <bitset>
#include "TGLib/TGLib.h"

class Keyboard {
	friend class Window;
public:
	class Event {
	public:
		enum class Type {
			Press, Release, Invalid
		};
	private:
		Type type;
		u8 code;
	public:
		Event() noexcept : type(Type::Invalid), code(0) {}
		Event(Type type, u8 code) noexcept : type(type), code(code) {}
		bool IsPress() const noexcept { return type == Type::Press; }
		bool IsRelease() const noexcept { return type == Type::Release; }
		bool IsValid() const noexcept { return type != Type::Invalid; }
		u8 GetCode() const noexcept { return code; }
	};
public:
	Keyboard() = default;
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator=(const Keyboard&) = delete;
	
	void Update() noexcept;

	bool KeyIsPressed(u8 code) const noexcept; // polls the state of the key
	bool KeyPressed(u8 code) const noexcept; // has the key been pressed this frame (or since the last call to Update())
	bool KeyReleased(u8 code) const noexcept; // has the key been released this frame (or since the last call to Update())
	Event ReadKey() noexcept;
	bool KeyIsEmpty() const noexcept;
	void ClearKey() noexcept;

	char ReadChar() noexcept;
	bool CharIsEmpty() const noexcept;
	void ClearChar() noexcept;
	void Clear() noexcept;

	void EnableAutorepeat() noexcept;
	void DisableAutorepeat() noexcept;
	bool AutorepeatIsEnabled() const noexcept;
private:
	void OnKeyPressed(u8 code) noexcept;
	void OnKeyReleased(u8 code) noexcept;
	void OnChar(char character) noexcept;
	void ClearState() noexcept;

	template<typename T> 
	static void TrimBuffer(std::queue<T>& buffer) noexcept {
		while (buffer.size() > bufferSize) buffer.pop();
	}
private:
	static constexpr u32 keyCount = 256;
	static constexpr u32 bufferSize = 16;
	bool autorepeatEnabled = false;
	std::bitset<keyCount> keyStates;
	std::bitset<keyCount> keyStatesPrev;
	std::queue<Event> keyBuffer;
	std::queue<char> charBuffer;
};