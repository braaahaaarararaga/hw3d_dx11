#pragma once
#include "RenderWindow.h"
#include "Graphics/Graphics.h"
#include "Input/Input.h"

class WindowContainer
{
public:
	WindowContainer();
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	RenderWindow render_window;
	Input input;
	Graphics gfx;

	std::vector<BYTE> rawBuffer;
};