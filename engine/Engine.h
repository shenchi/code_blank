#pragma once

#include "Common.h"

namespace tofu
{
	class NativeContext;
	class Module;
	class RenderingSystem;
	class ScriptingSystem;
	class PhysicsSystem;
	class InputSystem;
	class AudioManager;

	class Time
	{
	public:
		static float TotalTime;
		static float DeltaTime;
		static float PhysicsTotalTime;
		static float FixedDeltaTime;
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

	public:

		int32_t UnloadLevel();

		void SetPhysicsSimulationPaused(bool paused) { physicsSimulationPaused = paused; }

		bool IsPhysicsSimulationPaused() const { return physicsSimulationPaused; }

	private:

		int32_t Shutdown();

	private:
		NativeContext*		nativeContext;

		RenderingSystem*	renderingSystem;
		PhysicsSystem*		physicsSystem;
		InputSystem*		inputSystem;

		AudioManager*		audioManager;

		Module*				userModules[kMaxUserModules];
		uint32_t			numUserModules;

	private:
		int64_t				timeCounterFreq;
		int64_t				startTimeCounter;
		int64_t				lastTimeCounter;
		int64_t				currentTimeCounter;

		bool				physicsSimulationPaused;
	};
}