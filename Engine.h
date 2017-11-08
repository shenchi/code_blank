#pragma once

#include "Common.h"

namespace tofu
{
	class NativeContext;
	class Module;
	class RenderingSystem;
	class ScriptingSystem;
	class InputSystem;

	class Time
	{
	public:
		static float TotalTime;
		static float DeltaTime;
	};

	class Engine
	{
		SINGLETON_DECL(Engine)

	public:
		Engine();

		int32_t AddModule(Module* module);

		int32_t Init(const char* filename);

		int32_t Run();

		int32_t Quit();

	private:

		int32_t Shutdown();

	private:
		NativeContext*		nativeContext;

		RenderingSystem*	renderingSystem;
		InputSystem*		inputSystem;

		Module*				userModules[MAX_USER_MODULES];
		uint32_t			numUserModules;

	private:
		int64_t				timeCounterFreq;
		int64_t				startTimeCounter;
		int64_t				lastTimeCounter;
		int64_t				currentTimeCounter;
	};
}