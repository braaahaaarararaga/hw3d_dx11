#pragma once
#include <queue>
#include <optional>

class MouseClass
{
public:
	struct MousePoint
	{
		int x;
		int y;
	};
	class Event
	{
	public:
		enum Type
		{
			LPress,
			LRelease,
			RPress,
			RRelease,
			MPress,
			MRelease,
			WheelUp,
			WheelDown,
			Move,
			RAW_MOVE,
			Enter,
			Leave
		};
	private:
		Type type;
		int x;
		int y;
		bool leftIsPressed;
		bool rightIsPressed;
	public:
		Event(const Type type, const int x, const int y)
			:
			type(type),
			x(x),
			y(y)
		{
		}
		Event::Type GetType() const
		{
			return type;
		}
		MousePoint GetPos() const
		{
			return { x,y };
		}
		int GetPosX() const
		{
			return x;
		}
		int GetPosY() const
		{
			return y;
		}
		bool LeftIsPreesed() const
		{
			return leftIsPressed;
		}
		bool RightIsPreesed() const
		{
			return rightIsPressed;
		}
	};
public:
	void OnLeftPressed(int x, int y);
	void OnLeftReleased(int x, int y);
	void OnRightPressed(int x, int y);
	void OnRightReleased(int x, int y);
	void OnMiddlePressed(int x, int y);
	void OnMiddleReleased(int x, int y);
	void OnWheelUp(int x, int y);
	void OnWheelDown(int x, int y);
	void OnMouseMove(int x, int y);
	void OnMouseMoveRaw(int x, int y);

	bool IsLeftDown();
	bool IsMiddleDown();
	bool IsRightDown();

	int GetPosX();
	int GetPosY();
	MousePoint GetPos();

	bool EventBufferIsEmpty();
	std::optional<MouseClass::Event> ReadEvent();

private:
	std::queue<Event> eventBuffer;
	bool leftIsDown = false;
	bool rightIsDown = false;
	bool mbuttonDown = false;
	int x = 0;
	int y = 0;
};