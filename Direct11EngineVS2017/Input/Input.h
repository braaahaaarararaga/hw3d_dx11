#pragma once
#include "MouseClass.h"
#include "KeyboardClass.h"

class Input
{
public:
	Input();
	~Input();

public:
	MouseClass mouse;
	KeyboardClass keyboard;
};

