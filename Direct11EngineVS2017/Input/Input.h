#pragma once
#include "MouseClass.h"
#include "KeyboardClass.h"

class Input
{
public:
	Input();
	~Input();
	void Update();

private:

public:
	MouseClass mouse;
	KeyboardClass keyboard;
};

