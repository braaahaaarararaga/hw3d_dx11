#include "Engine.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR	lpCmdLine,
	_In_ int	nCmdShow)
{

	Engine engine;
	try
	{
		 engine.Initialize(hInstance, "Title", "MyWindowClass", 1200, 900);
	}
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return 0;
	}
	while (engine.ProcessMessages() == true)
	{
		engine.Update();
		engine.RenderFrame();
	}
	return 0;
}

