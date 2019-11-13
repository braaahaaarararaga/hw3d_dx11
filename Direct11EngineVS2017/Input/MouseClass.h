#pragma once
#include <queue>
#include <optional>

class MouseClass
{
	friend class WindowContainer;
	enum class MouseButton
	{
		Left,
		Right,
		Middle,
		X1,
		X2,
		MAX_MOUSE_BUTTONS
	};
public:
	struct MousePoint
	{
		int x;
		int y;
	};


	MouseClass() = default;
	MouseClass(const MouseClass&) = delete;
	MouseClass& operator=(const MouseClass&) = delete;

	std::optional<MousePoint> ReadRawData();
	bool IsMouseButtonDown(const MouseButton mb) const;
	bool IsLeftDown() const;
	bool IsMiddleDown() const;
	bool IsRightDown() const;
	bool IsX1Down() const;
	bool IsX2Down() const;
	bool IsMouseInWindow();
	bool IsRawBufferEmpty();
	bool RawEnabled();
	void EnableRawInput();
	void DisableRawInput();

	int GetPosX();
	int GetPosY();
	MousePoint GetPos();

	void FillRawPoint();
	MousePoint rawPoint;
private:
	void OnMouseButtonDown(const int x, const int y, const MouseButton mb);
	void OnMouseButtonUp(const int x, const int y, const MouseButton mb);
	void OnWheelDelta(const int x, const int y, const float delta);
	void OnMouseMove(int x, int y);
	void OnMouseEnter();
	void OnMouseLeave();
	void OnMouseMoveRaw(int x, int y);

private:
	int x = 0;
	int y = 0;
	bool mouseInWindow = true;
	bool rawInputEnabled = true;
	bool mouseButtonStates[(int)MouseButton::MAX_MOUSE_BUTTONS] = {0};
	float wheelDeltaCarry = 0.0f;
	int wheelDelta = 0;
	std::queue<MousePoint> rawDeltaBuffer;
};