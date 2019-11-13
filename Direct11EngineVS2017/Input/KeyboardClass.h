#pragma once
#include <queue>
#include <optional>

class KeyboardClass
{
	friend class WindowContainer;
public:
	class Event
	{
	public:
		enum Type
		{
			Press,
			Release
		};

		Event(const Type type, const unsigned char key)
			:
			type(type),
			key(key)
		{}
		bool IsPress() const
		{
			return this->type == Type::Press;
		}
		bool IsRelease() const
		{
			return this->type == Type::Release;
		}
		unsigned char GetKeyCode() const
		{
			return this->key;
		}

	private:
		Type type;
		unsigned char key;
	};
public:
	KeyboardClass();
	bool KeyIsPressed(const unsigned char keycode);
	bool CharBufferIsEmpty();
	std::optional<unsigned char> Readchar();
	void EnableAutoRepeatKeys();
	void DisableAutoRepeatKeys();
	void EnableAutoRepeatChars();
	void DisableAutoRepeatChars();
	bool IsKeyAutoRepeat();
	bool IsCharAutoRepeat();
	void FlushChar();
private:
	void OnKeyPressed(const unsigned char key);
	void OnKeyRelease(const unsigned char key);
	void OnChar(const unsigned char key);

private:
	bool autoRepeatKeys = false;
	bool autoRepeatChars = false;
	bool keyStates[256];
	std::queue<unsigned char> charBuffer;
};