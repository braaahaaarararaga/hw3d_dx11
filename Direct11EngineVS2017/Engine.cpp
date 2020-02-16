#include "Engine.h"
#include "ResourceManager.h"

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
	input.Update();
	
	if (input.mouse.IsRightDown())
	{
		this->gfx.camera3D.AdjustRotation(input.mouse.rawPoint.x * 0.01f, input.mouse.rawPoint.y * 0.01, 0.0f);
	}

	float Camera3DSpeed = 0.005f;

	if (input.keyboard.KeyIsPressed(VK_SHIFT))
	{
		Camera3DSpeed = 0.05f;
	}

	if (input.keyboard.KeyIsPressed('W'))
	{
		this->gfx.camera3D.AdjustPosition(this->gfx.camera3D.GetForwardVector() * Camera3DSpeed * dt);
	}
	if (input.keyboard.KeyIsPressed('S'))
	{
		this->gfx.camera3D.AdjustPosition(this->gfx.camera3D.GetBackwardVector() * Camera3DSpeed * dt);
	}
	if (input.keyboard.KeyIsPressed('A'))
	{
		this->gfx.camera3D.AdjustPosition(this->gfx.camera3D.GetLeftVector() * Camera3DSpeed * dt);
	}
	if (input.keyboard.KeyIsPressed('D'))
	{
		this->gfx.camera3D.AdjustPosition(this->gfx.camera3D.GetRightVector() * Camera3DSpeed * dt);
	}
	if (input.keyboard.KeyIsPressed(VK_SPACE))
	{
		this->gfx.camera3D.AdjustPosition(0.0f, Camera3DSpeed * dt, 0.0f);
	}
	if (input.keyboard.KeyIsPressed(VK_CONTROL))
	{
		this->gfx.camera3D.AdjustPosition(0.0f, -Camera3DSpeed * dt, 0.0f);
	}
	if (input.keyboard.KeyIsPressed('C'))
	{
		XMVECTOR lightPosition = this->gfx.camera3D.GetPositionVector();
		lightPosition += this->gfx.camera3D.GetForwardVector();
		this->gfx.light.SetPosition(lightPosition);
		this->gfx.light.SetRotation(this->gfx.camera3D.GetRotationFloat3());
	}
}

void Engine::RenderFrame()
{
	gfx.RenderBegin();
	gfx.SetLight();
	gfx.RenderShadowMap();

	gfx.RenderFrame();

	gfx.RenderText();
	gfx.RenderImGui();
	gfx.RenderEnd();
}

void Engine::Finalize()
{
	ResourceManager::CleanUp();
}
