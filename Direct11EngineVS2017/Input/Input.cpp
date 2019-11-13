#include "Input.h"



Input::Input()
{
}


Input::~Input()
{
}

void Input::Update()
{
	// input check
	while (!keyboard.CharBufferIsEmpty())
	{
		auto opt = keyboard.Readchar();
		if (opt.has_value())
			unsigned char ch = opt.value();
	}
	while (!keyboard.KeyBufferIsEmpty())
	{
		auto opt = keyboard.ReadKey();
		if (opt.has_value())
		{
			auto kbe = opt.value();
			unsigned char Keycode = kbe.GetKeyCode();
		}
	}
}
