#include "KeyboardClass.h"

KeyboardClass::KeyboardClass()
{
	for (int i = 0; i < 256; i++)
	{
		this->keyStates[i] = false;
	}
}

bool KeyboardClass::KeyIsPressed(const unsigned char keycode)
{
	return this->keyStates[keycode];
}

bool KeyboardClass::KeyBufferIsEmpty()
{
	return this->keyBuffer.empty();
}

bool KeyboardClass::CharBufferIsEmpty()
{
	return this->charBuffer.empty();
}

std::optional<KeyboardClass::Event> KeyboardClass::ReadKey()
{
	if(keyBuffer.size() > 0u)
	{
		Event e = this->keyBuffer.front(); // Get first Keyboard Event from queue
		this->keyBuffer.pop(); // remove first item from queue
		return e; // return keyboard event
	}
	return {};
}

std::optional<unsigned char> KeyboardClass::Readchar()
{
	if (charBuffer.size() > 0u)
	{
		unsigned char e = this->charBuffer.front();
		charBuffer.pop();
		return e;
	}
	return {};
}

void KeyboardClass::OnKeyPressed(const unsigned char key)
{
	this->keyStates[key] = true;
	this->keyBuffer.push(Event(Event::Type::Press, key));
}

void KeyboardClass::OnKeyRelease(const unsigned char key)
{
	this->keyStates[key] = false;
	this->keyBuffer.push(Event(Event::Type::Release, key));
}

void KeyboardClass::OnChar(const unsigned char key)
{
	this->charBuffer.push(key);
}

void KeyboardClass::EnableAutoRepeatKeys()
{
	autoRepeatKeys = true;
}

void KeyboardClass::DisableAutoRepeatKeys()
{
	autoRepeatKeys = false;
}

void KeyboardClass::EnableAutoRepeatChars()
{
	autoRepeatChars = true;
}

void KeyboardClass::DisableAutoRepeatChars()
{
	autoRepeatChars = false;
}

bool KeyboardClass::IsKeyAutoRepeat()
{
	return autoRepeatKeys;
}

bool KeyboardClass::IsCharAutoRepeat()
{
	return autoRepeatChars;
}
