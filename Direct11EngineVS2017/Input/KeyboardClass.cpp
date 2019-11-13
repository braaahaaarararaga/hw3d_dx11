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


bool KeyboardClass::CharBufferIsEmpty()
{
	return this->charBuffer.empty();
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
}

void KeyboardClass::OnKeyRelease(const unsigned char key)
{
	this->keyStates[key] = false;
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

void KeyboardClass::FlushChar()
{
	charBuffer = std::queue<unsigned char>();
}
