#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>
#endif

#include "..\engine\Engine.h"
#include <Windows.h>
#include <cassert>

#include "Game.h"
#include "SceneLoadingDemo.h"
//#include "CubemapProbeDemo.h"
#include "AnimationDemo.h"

using tofu::kOK;

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetBreakAlloc(132852);
#endif

	tofu::Engine engine;
	CHECKED(engine.Init("config.lua"));
	CHECKED(engine.AddModule(new SceneLoadingDemo()));
	//CHECKED(engine.AddModule(new Game()));
	//CHECKED(engine.AddModule(new AnimationDemo()));

	return engine.Run();
}
