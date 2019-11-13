#include "MouseClass.h"

void MouseClass::OnLeftPressed(int x, int y)
{
	leftIsDown = true;
	eventBuffer.push(MouseClass::Event(MouseClass::Event::Type::LPress, x, y));
}

void MouseClass::OnLeftReleased(int x, int y)
{
	leftIsDown = false;
	eventBuffer.push(MouseClass::Event(MouseClass::Event::Type::LRelease, x, y));
}

void MouseClass::OnRightPressed(int x, int y)
{
	rightIsDown = true;
	eventBuffer.push(MouseClass::Event(MouseClass::Event::Type::RPress, x, y));
}

void MouseClass::OnRightReleased(int x, int y)
{
	rightIsDown = false;
	eventBuffer.push(MouseClass::Event(MouseClass::Event::Type::RRelease, x, y));
}

void MouseClass::OnMiddlePressed(int x, int y)
{
	mbuttonDown = true;
	eventBuffer.push(MouseClass::Event(MouseClass::Event::Type::MPress, x, y));
}

void MouseClass::OnMiddleReleased(int x, int y)
{
	mbuttonDown = false;
	eventBuffer.push(MouseClass::Event(MouseClass::Event::Type::MRelease, x, y));
}

void MouseClass::OnWheelUp(int x, int y)
{
	eventBuffer.push(MouseClass::Event(MouseClass::Event::Type::WheelUp, x, y));
}

void MouseClass::OnWheelDown(int x, int y)
{
	eventBuffer.push(MouseClass::Event(MouseClass::Event::Type::WheelDown, x, y));
}

void MouseClass::OnMouseMove(int x, int y)
{
	this->x = x;
	this->y = y;
	eventBuffer.push(MouseClass::Event(MouseClass::Event::Type::Move, x, y));
}

void MouseClass::OnMouseMoveRaw(int x, int y)
{
	this->eventBuffer.push(MouseClass::Event(MouseClass::Event::RAW_MOVE, x, y));
}

bool MouseClass::IsLeftDown()
{
	return leftIsDown;
}

bool MouseClass::IsMiddleDown()
{
	return mbuttonDown;
}

bool MouseClass::IsRightDown()
{
	return rightIsDown;
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

bool MouseClass::EventBufferIsEmpty()
{
	return eventBuffer.empty();
}

std::optional<MouseClass::Event> MouseClass::ReadEvent()
{
	if (eventBuffer.size() > 0u)
	{
		MouseClass::Event me = eventBuffer.front();
		eventBuffer.pop();
		return me;
	}
	return {};
}
