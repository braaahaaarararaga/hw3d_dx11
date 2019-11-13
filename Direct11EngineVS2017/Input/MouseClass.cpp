#include "MouseClass.h"
/* Value for rolling one detent */
#define _WHEEL_DELTA 120

void MouseClass::OnMouseButtonDown(const int x, const int y, const MouseButton mb)
{
	this->x = x;
	this->y = y;
	mouseButtonStates[(int)mb] = true;
}

void MouseClass::OnMouseButtonUp(const int x, const int y, const MouseButton mb)
{
	this->x = x;
	this->y = y;
	mouseButtonStates[(int)mb] = false;
}

void MouseClass::OnWheelDelta(const int x, const int y, const float delta)
{
	wheelDeltaCarry += delta;
	while (wheelDeltaCarry >= _WHEEL_DELTA)
	{
		wheelDeltaCarry -= _WHEEL_DELTA;
		wheelDelta--;
	}
	while (wheelDeltaCarry <= -_WHEEL_DELTA)
	{
		wheelDeltaCarry += _WHEEL_DELTA;
		wheelDelta++;
	}
}

void MouseClass::OnMouseMove(int x, int y)
{
	this->x = x;
	this->y = y;
}

void MouseClass::OnMouseEnter()
{
	mouseInWindow = true;
}

void MouseClass::OnMouseLeave()
{
	mouseInWindow = false;
}

void MouseClass::OnMouseMoveRaw(int x, int y)
{
	rawDeltaBuffer.push({ y,x });
}

std::optional<MouseClass::MousePoint> MouseClass::ReadRawData()
{
	if (rawDeltaBuffer.empty())
	{
		return {};
	}
	const MousePoint pt = rawDeltaBuffer.front();
	rawDeltaBuffer.pop();
	return pt;
}

bool MouseClass::IsMouseButtonDown(const MouseButton mb) const
{
	return mouseButtonStates[(int)mb];
}

bool MouseClass::IsLeftDown() const
{
	return mouseButtonStates[(int)MouseButton::Left];
}

bool MouseClass::IsMiddleDown() const
{
	return mouseButtonStates[(int)MouseButton::Middle];
}

bool MouseClass::IsRightDown() const
{
	return mouseButtonStates[(int)MouseButton::Right];
}

bool MouseClass::IsX1Down() const
{
	return mouseButtonStates[(int)MouseButton::X1];
}

bool MouseClass::IsX2Down() const
{
	return mouseButtonStates[(int)MouseButton::X2];
}

bool MouseClass::IsMouseInWindow()
{
	return mouseInWindow;
}

bool MouseClass::IsRawBufferEmpty()
{
	return rawDeltaBuffer.empty();
}

bool MouseClass::RawEnabled()
{
	return rawInputEnabled;
}

void MouseClass::EnableRawInput()
{
	rawInputEnabled = true;
}

void MouseClass::DisableRawInput()
{
	rawInputEnabled = false;
}

int MouseClass::GetPosX()
{
	return x;
}

int MouseClass::GetPosY()
{
	return y;
}

MouseClass::MousePoint MouseClass::GetPos()
{
	return { x,y };
}

void MouseClass::FillRawPoint()
{
	rawPoint = { 0 };
	while (!IsRawBufferEmpty())
	{
		auto opt = ReadRawData();
		if (opt.has_value())
		{
			auto rawData = opt.value();
			rawPoint.x += rawData.x;
			rawPoint.y += rawData.y;
		}
	}
}
