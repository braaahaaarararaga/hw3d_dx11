#include "Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	timer.Start();
	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
	{
		return false;
	}

	if (!gfx.Initialize(this->render_window.GetHWND(), width, height))
		return false;
	return true;
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update()
{
	float dt = timer.GetMiliseceondsElapsed();
	timer.Restart();

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

	while (!mouse.EventBufferIsEmpty())
	{
		auto opt = mouse.ReadEvent();
		if (!opt.has_value())
			continue;
		MouseClass::Event me = opt.value();
		if (me.GetType() == MouseClass::Event::Type::WheelUp)
		{
			OutputDebugString(L"wheel up\n");
		}
		if (me.GetType() == MouseClass::Event::Type::WheelDown)
		{
			OutputDebugStringA("wheel down\n");
		}
		if (mouse.IsRightDown())
		{
			if (me.GetType() == MouseClass::Event::Type::RAW_MOVE)
			{
				this->gfx.Camera3D.AdjustRotation((float)me.GetPosY() * 0.01f, (float)me.GetPosX() * 0.01, 0.0f);
			}
		}
	}

	float Camera3DSpeed = 0.005f;

	if (keyboard.KeyIsPressed(VK_SHIFT))
	{
		Camera3DSpeed = 0.5f;
	}

	if (keyboard.KeyIsPressed('W'))
	{
		this->gfx.Camera3D.AdjustPosition(this->gfx.Camera3D.GetForwardVector() * Camera3DSpeed * dt);
	}
	if (keyboard.KeyIsPressed('S'))
	{
		this->gfx.Camera3D.AdjustPosition(this->gfx.Camera3D.GetBackwardVector() * Camera3DSpeed * dt);
	}
	if (keyboard.KeyIsPressed('A'))
	{
		this->gfx.Camera3D.AdjustPosition(this->gfx.Camera3D.GetLeftVector() * Camera3DSpeed * dt);
	}
	if (keyboard.KeyIsPressed('D'))
	{
		this->gfx.Camera3D.AdjustPosition(this->gfx.Camera3D.GetRightVector() * Camera3DSpeed * dt);
	}
	if (keyboard.KeyIsPressed(VK_SPACE))
	{
		this->gfx.Camera3D.AdjustPosition(0.0f, Camera3DSpeed * dt, 0.0f);
	}
	if (keyboard.KeyIsPressed(VK_CONTROL))
	{
		this->gfx.Camera3D.AdjustPosition(0.0f, -Camera3DSpeed * dt, 0.0f);
	}
	if (keyboard.KeyIsPressed('C'))
	{
		XMVECTOR lightPosition = this->gfx.Camera3D.GetPositionVector();
		lightPosition += this->gfx.Camera3D.GetForwardVector();
		this->gfx.light.SetPosition(lightPosition);
		this->gfx.light.SetRotation(this->gfx.Camera3D.GetRotationFloat3());
	}
}

void Engine::RenderFrame()
{
	gfx.RenderBegin();
	gfx.SetLight();

	gfx.RenderFrame();

	gfx.RenderText();
	gfx.RenderImGui();
	gfx.RenderEnd();
}
