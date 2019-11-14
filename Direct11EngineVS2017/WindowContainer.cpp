#include "WindowContainer.h"

WindowContainer::WindowContainer()
{
	static bool raw_input_initialized = false;
	if (raw_input_initialized == false)
	{
		RAWINPUTDEVICE rid;

		rid.usUsagePage = 0x01; // Mouse. See: https://docs.microsoft.com/en-us/windows-hardware/drivers/hid/hidclass-hardware-ids-for-top-level-collections
		rid.usUsage = 0x02;
		rid.dwFlags = 0;
		rid.hwndTarget = NULL; // Same as keyboard focus

		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
		{
			ErrorLogger::Log(GetLastError(), "Failed to register raw input devices");
			exit(-1);
			// registration failed. Call GetLastError for the cause of the error
		}

		raw_input_initialized = true;
	}
}


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WindowContainer::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	// Keyboard messages
#pragma region Keyboard Messages
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		unsigned char keycode = static_cast<unsigned char>(wParam);
		if (input.keyboard.IsKeyAutoRepeat())
		{
			input.keyboard.OnKeyPressed(keycode);
		}
		else
		{
			const bool wasPressed = lParam & 0x40000000;
			if (!wasPressed)
			{
				input.keyboard.OnKeyPressed(keycode);
			}
		}
		return 0;
	}
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		unsigned char keycode = static_cast<unsigned char>(wParam);
		input.keyboard.OnKeyRelease(keycode);
		return 0;
	}
	case WM_CHAR:
	{
		unsigned char ch = static_cast<unsigned char>(wParam);
		if (input.keyboard.IsCharAutoRepeat())
		{
			input.keyboard.OnChar(ch);
		}
		else
		{
			const bool wasPressed = lParam & 0x40000000;
			if (!wasPressed)
			{
				input.keyboard.OnChar(wParam);
			}
		}
		return 0;
	}
#pragma endregion
	// Mouse Messages
	case WM_MOUSEMOVE:
	{
		POINTS pos = MAKEPOINTS(lParam);
		if (pos.x >= 0 && pos.x < render_window.GetWidht() && pos.y >= 0 && pos.y < render_window.GetHeight())
		{
			input.mouse.OnMouseMove(pos.x, pos.y);
			if (!input.mouse.IsMouseInWindow())
			{
				SetCapture(render_window.GetHWND());
				input.mouse.OnMouseEnter();
			}
		}
		else
		{
			if (wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON | MK_XBUTTON1 | MK_XBUTTON2))
			{
				input.mouse.OnMouseMove(pos.x, pos.y);
			}
			else
			{
				ReleaseCapture();
				input.mouse.OnMouseLeave();
			}
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		input.mouse.OnMouseButtonDown(x, y, MouseClass::MouseButton::Left);
		return 0;
	}
	case WM_LBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		input.mouse.OnMouseButtonUp(x, y, MouseClass::MouseButton::Left);
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		input.mouse.OnMouseButtonDown(x, y, MouseClass::MouseButton::Right);
		return 0;
	}
	case WM_RBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		input.mouse.OnMouseButtonUp(x, y, MouseClass::MouseButton::Right);
		return 0;
	}
	case WM_MBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		input.mouse.OnMouseButtonDown(x, y, MouseClass::MouseButton::Middle);
		return 0;
	}
	case WM_MBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		input.mouse.OnMouseButtonUp(x, y, MouseClass::MouseButton::Middle);
		return 0;
	}
	case WM_XBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
		{
			input.mouse.OnMouseButtonDown(x, y, MouseClass::MouseButton::X1);
		}
		else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2)
		{
			input.mouse.OnMouseButtonDown(x, y, MouseClass::MouseButton::X2);
		}
		break;
	}
	case WM_XBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
		{
			input.mouse.OnMouseButtonUp(x, y, MouseClass::MouseButton::X1);
		}
		else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2)
		{
			input.mouse.OnMouseButtonUp(x, y, MouseClass::MouseButton::X2);
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		input.mouse.OnWheelDelta(x, y, delta);
		break;
	}
	case WM_INPUT:
	{
		if (!input.mouse.RawEnabled())
		{
			break;
		}

		UINT size;
		// first get the size of the input data
		if (GetRawInputData(
			reinterpret_cast<HRAWINPUT>(lParam),
			RID_INPUT,
			nullptr,
			&size,
			sizeof(RAWINPUTHEADER)) == -1)
		{
			// bail msg processing if error
			break;
		}
		rawBuffer.resize(size);
		// read in the input data
		if (GetRawInputData(
			reinterpret_cast<HRAWINPUT>(lParam),
			RID_INPUT,
			rawBuffer.data(),
			&size,
			sizeof(RAWINPUTHEADER)) != size)
		{
			// bail msg processing if error
			break;
		}
		// process the raw input data
		auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
		if (ri.header.dwType == RIM_TYPEMOUSE &&
			(ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
		{
			/*std::wostringstream oss;
			oss << "(" << ri.data.mouse.lLastX << "," << ri.data.mouse.lLastY << ")";
			SetWindowText(render_window.GetHWND(), oss.str().c_str());*/
			input.mouse.OnMouseMoveRaw(ri.data.mouse.lLastX, ri.data.mouse.lLastY);
		}
		break;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
