#include "Input.h"



Input::Input()
{
}


Input::~Input()
{
}

void Input::Update()
{
	// type check
	while (!keyboard.CharBufferIsEmpty())
	{
		auto opt = keyboard.Readchar();
		if (opt.has_value())
			unsigned char ch = opt.value();
	}
	mouse.FillRawPoint();
}
