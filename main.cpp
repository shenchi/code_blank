#include "Engine.h"
#include <Windows.h>
#include <cassert>

#include "TestGame.h"

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	tofu::Engine engine;
	assert(tofu::TF_OK == engine.Init("config.lua"));
	assert(tofu::TF_OK == engine.AddModule(new TestGame()));
	return engine.Run();
}
