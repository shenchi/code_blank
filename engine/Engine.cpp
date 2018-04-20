#include "Engine.h"

#include <cassert>

#include "NativeContext.h"

#include "RenderingSystem.h"
#include "PhysicsSystem.h"
#include "InputSystem.h"
#include "AudioManager.h"
#include "MemoryAllocator.h"

#include "AnimationComponent.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "PhysicsComponent.h"
#include "RenderingComponent.h"
#include "TransformComponent.h"

#include "PerformanceTimer.h"

namespace tofu
{
	float Time::TotalTime = 0.0f;
	float Time::DeltaTime = 0.0f;
	float Time::PhysicsTotalTime = 0.0f;
	float Time::FixedDeltaTime = kDefaultFixedDeltaTime;

	SINGLETON_IMPL(Engine);

	Engine::Engine()
		:
		nativeContext(nullptr),
		renderingSystem(nullptr),
		inputSystem(nullptr),
		audioManager(nullptr),
		userModules(),
		numUserModules(0),
		timeCounterFreq(0),
		startTimeCounter(0),
		lastTimeCounter(0),
		currentTimeCounter(0)
	{
		assert(nullptr == _instance);
		_instance = this;
	}

	int32_t Engine::AddModule(Module * module)
	{
		if (numUserModules >= kMaxUserModules)
			return kErrUnknown;

		userModules[numUserModules] = module;
		numUserModules++;

		return module->Init();
	}

	int32_t Engine::Init(const char* filename)
	{
		// initialize native context
		nativeContext = NativeContext::Create();
		if (nullptr == nativeContext)
			return kErrUnknown;
		int32_t err = nativeContext->Init();
		CHECKED(err);

		// Initialize Memory Management for Rendering
		CHECKED(MemoryAllocator::Allocators[kAllocGlobal].Init(
			kGlobalMemSize,
			kGlobalMemAlign));

		CHECKED(MemoryAllocator::Allocators[kAllocLevel].Init(
			kLevelMemSize,
			kLevelMemAlign));

		for (uint32_t i = kAllocFrame;
			i <= kAllocFrameEnd;
			++i)
		{
			CHECKED(MemoryAllocator::Allocators[i].Init(
				kFrameMemSize,
				kFrameMemAlign));
		}

		MemoryAllocator::currentFrameAllocIdx = 0;

		// init entity system
		Entity::RegisterComponent<AnimationComponent>();
		Entity::RegisterComponent<CameraComponent>();
		Entity::RegisterComponent<LightComponent>();
		Entity::RegisterComponent<PhysicsComponent>();
		Entity::RegisterComponent<RenderingComponent>();
		Entity::RegisterComponent<TransformComponent>();
		CHECKED(Entity::Init());

		// initialize rendering system
		renderingSystem = new RenderingSystem();
		CHECKED(renderingSystem->Init());

		// initialize physics system
		physicsSystem = new PhysicsSystem();
		CHECKED(physicsSystem->Init());

		// initialize input system
		inputSystem = new InputSystem();
		CHECKED(inputSystem->Init());

		audioManager = new AudioManager();
		CHECKED(audioManager->Init());

		Time::FixedDeltaTime = kDefaultFixedDeltaTime;

		return kOK;
	}

	int32_t Engine::Run()
	{
		PERFORMANCE_TIMER_INIT();

		// keep performance counter on start
		timeCounterFreq = nativeContext->GetTimeCounterFrequency();
		startTimeCounter = nativeContext->GetTimeCounter();
		currentTimeCounter = startTimeCounter;
		lastTimeCounter = currentTimeCounter;

		Time::TotalTime = 0.0f;
		Time::PhysicsTotalTime = 0.0f;

		char textBuf[1024] = {};

		// process native event and see if we need to quit
		while(nativeContext->ProcessEvent())
		{
			// update timing data
			currentTimeCounter = nativeContext->GetTimeCounter();

			int64_t deltaTimeCounter = (currentTimeCounter - lastTimeCounter) * 1000000;
			float deltaTime = (deltaTimeCounter / timeCounterFreq) / 1000000.0f;

			// lock to 60 fps
			if (deltaTime < 0.016f) continue;

#if TOFU_PERFORMANCE_TIMER_ENABLED == 1
			float cpuTime = PerformanceTimer::GetTime(PerformanceTimer::deltaTimerSlots[kPerformanceTimerSlotFrameTime]);
			float physicsTime = PerformanceTimer::GetTime(PerformanceTimer::deltaTimerSlots[kPerformanceTimerSlotPhysicsTime]);
			float userModuleTime = PerformanceTimer::GetTime(PerformanceTimer::deltaTimerSlots[kPerformanceTimerSlotUserUpdateTime]);
			float renderingSystemTime = PerformanceTimer::GetTime(PerformanceTimer::deltaTimerSlots[kPerformanceTimerSlotRenderingSystemTime]);
			float customMeasuringTime = PerformanceTimer::GetTime(PerformanceTimer::deltaTimerSlots[kPerformanceTimerSlotCustem]);
#endif

			PERFORMANCE_TIMER_START(kPerformanceTimerSlotFrameTime);

			Time::DeltaTime = deltaTime;
			Time::TotalTime += Time::DeltaTime;

			lastTimeCounter = currentTimeCounter;

			// a new frame start

			MemoryAllocator::SwapFrameAllocator();

			CHECKED(renderingSystem->BeginFrame());

			CHECKED(inputSystem->Update());

			CHECKED(audioManager->Update());

			PERFORMANCE_TIMER_START(kPerformanceTimerSlotPhysicsTime);

			CHECKED(physicsSystem->PreUpdate());

			// TODO: we put the clean up here to
			// wait physics system delete rigibodies
			Entity::CleanupComponents();

			float phyTime = Time::PhysicsTotalTime;
			uint32_t count = 0;

			while (phyTime + Time::FixedDeltaTime <= Time::TotalTime)
			{
				CHECKED(physicsSystem->Update());
				phyTime += Time::FixedDeltaTime;
				Time::PhysicsTotalTime = phyTime;

				for (uint32_t i = 0; i < numUserModules; i++)
				{
					CHECKED(userModules[i]->FixedUpdate());
				}

				count++;
				//if (count >= kMaxPhysicsStepsPerFrame) break;
			}

			CHECKED(physicsSystem->PostUpdate());

			PERFORMANCE_TIMER_END(kPerformanceTimerSlotPhysicsTime);

			PERFORMANCE_TIMER_START(kPerformanceTimerSlotUserUpdateTime);

			for (uint32_t i = 0; i < numUserModules; i++)
			{
				CHECKED(userModules[i]->Update());
			}

			PERFORMANCE_TIMER_END(kPerformanceTimerSlotUserUpdateTime);

			PERFORMANCE_TIMER_START(kPerformanceTimerSlotRenderingSystemTime);

#if TOFU_PERFORMANCE_TIMER_ENABLED == 1
			GUI* gui = GUI::instance();

			gui->SetCanvasSize(1920, 1080);

			uint32_t layer = kMaxGUILayers - 1;
			gui->SetupLayer(layer);

			math::float4 white(1, 1, 1, 1);

			sprintf_s(textBuf, "CPU Time: %.2f ms", cpuTime);
			gui->Text(layer, -950, -540, 24, textBuf, white, kTextAlignTop);

			sprintf_s(textBuf, "Physics Time: %.2f ms", physicsTime);
			gui->Text(layer, -950, -520, 24, textBuf, white, kTextAlignTop);

			sprintf_s(textBuf, "Gameplay Logic Time: %.2f ms", userModuleTime);
			gui->Text(layer, -950, -500, 24, textBuf, white, kTextAlignTop);

			sprintf_s(textBuf, "Rendering System Time: %.2f ms", renderingSystemTime);
			gui->Text(layer, -950, -480, 24, textBuf, white, kTextAlignTop);

			sprintf_s(textBuf, "GPU Time: %.2f ms", renderingSystem->GetGPUTime());
			gui->Text(layer, -950, -460, 24, textBuf, white, kTextAlignTop);

			sprintf_s(textBuf, "Custom Measuring Time: %.2f ms", customMeasuringTime);
			gui->Text(layer, -950, -440, 24, textBuf, white, kTextAlignTop);

			sprintf_s(textBuf, "Phys and Gameplay Time Diff: %.2f ms", Time::TotalTime - Time::PhysicsTotalTime);
			gui->Text(layer, -950, -420, 24, textBuf, white, kTextAlignTop);
#endif

			PERFORMANCE_TIMER_START(kPerformanceTimerSlotCustem);
			PERFORMANCE_TIMER_PAUSE(kPerformanceTimerSlotCustem);

			CHECKED(renderingSystem->Update());

			CHECKED(renderingSystem->EndFrame());

			PERFORMANCE_TIMER_RESUME(kPerformanceTimerSlotCustem);
			PERFORMANCE_TIMER_END(kPerformanceTimerSlotCustem);

			PERFORMANCE_TIMER_END(kPerformanceTimerSlotRenderingSystemTime);

			PERFORMANCE_TIMER_END(kPerformanceTimerSlotFrameTime);

			// frame ends

			CHECKED(renderingSystem->SwapBuffers());
		}

		CHECKED(Shutdown());
		return kOK;
	}

	int32_t Engine::Quit()
	{
		return nativeContext->QuitApplication();
	}

	int32_t Engine::UnloadLevel()
	{
		CHECKED(Entity::Reset());
		CHECKED(renderingSystem->CleanupLevelResources());
		MemoryAllocator::Allocators[kAllocLevel].Reset();
		return kOK;
	}

	int32_t Engine::Shutdown()
	{
		for (uint32_t i = 0; i < numUserModules; i++)
		{
			CHECKED(userModules[i]->Shutdown());
			delete userModules[i];
		}

		CHECKED(inputSystem->Shutdown());
		delete inputSystem;

		CHECKED(physicsSystem->Shutdown());
		delete physicsSystem;

		CHECKED(renderingSystem->Shutdown());
		delete renderingSystem;

		CHECKED(audioManager->Shutdown());
		delete audioManager;

		CHECKED(nativeContext->Shutdown());
		delete nativeContext;

		CHECKED(Entity::Shutdown());

		// shutdown memory allocators

		for (uint32_t i = kAllocFrame;
			i <= kAllocFrameEnd;
			++i)
		{
			CHECKED(MemoryAllocator::Allocators[i].Shutdown());
		}

		CHECKED(MemoryAllocator::Allocators[kAllocLevel].Shutdown());

		CHECKED(MemoryAllocator::Allocators[kAllocGlobal].Shutdown());

		return kOK;
	}
}
