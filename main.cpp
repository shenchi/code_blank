#include "Engine.h"
#include <Windows.h>


int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	tofu::Engine engine;
	engine.Init(L"config.lua");
	return engine.Run();
}
