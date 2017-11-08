#include "Engine.h"

#include <cassert>

#include "NativeContext.h"

#include "RenderingSystem.h"

#include "InputSystem.h"

namespace tofu
{
	float Time::TotalTime = 0.0f;
	float Time::DeltaTime = 0.0f;

	SINGLETON_IMPL(Engine);

	Engine::Engine()
		:
		nativeContext(nullptr),
		renderingSystem(nullptr),
		inputSystem(nullptr),
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
		if (numUserModules >= MAX_USER_MODULES)
			return TF_UNKNOWN_ERR;

		userModules[numUserModules] = module;
		numUserModules++;

		return module->Init();
	}

	int32_t Engine::Init(const char* filename)
	{
		int32_t err = TF_OK;

		// initialize native context

		nativeContext = NativeContext::Create();
		if (nullptr == nativeContext)
			return TF_UNKNOWN_ERR;

		err = nativeContext->Init();
		if (TF_OK != err)
			return err;

		// initialize rendering system

		renderingSystem = new RenderingSystem();
		err = renderingSystem->Init();
		if (TF_OK != err)
			return err;

		inputSystem = new InputSystem();
		err = inputSystem->Init();
		if (TF_OK != err)
			return err;

		return err;
	}

	int32_t Engine::Run()
	{
		int32_t err = TF_OK;

		timeCounterFreq = nativeContext->GetTimeCounterFrequency();
		startTimeCounter = nativeContext->GetTimeCounter();
		currentTimeCounter = startTimeCounter;
		lastTimeCounter = currentTimeCounter;

		while(nativeContext->ProcessEvent())
		{
			currentTimeCounter = nativeContext->GetTimeCounter();

			int64_t deltaTimeCounter = (currentTimeCounter - lastTimeCounter) * 1000000;
			float deltaTime = (deltaTimeCounter / timeCounterFreq) / 1000000.0f;

			if (deltaTime < 0.016f) continue;

			Time::DeltaTime = deltaTime;
			Time::TotalTime += Time::DeltaTime;

			lastTimeCounter = currentTimeCounter;

			renderingSystem->BeginFrame();

			inputSystem->Update();

			for (uint32_t i = 0; i < numUserModules; i++)
			{
				err = userModules[i]->Update();
				if (TF_OK != err)
					return err;
			}

			renderingSystem->Update();

			renderingSystem->EndFrame();
		}

		err = Shutdown();
		return err;
	}

	int32_t Engine::Quit()
	{
		return nativeContext->QuitApplication();
	}

	int32_t Engine::Shutdown()
	{
		for (uint32_t i = 0; i < numUserModules; i++)
		{
			assert(TF_OK == userModules[i]->Shutdown());
			delete userModules[i];
		}

		assert(TF_OK == inputSystem->Shutdown());
		delete inputSystem;

		assert(TF_OK == renderingSystem->Shutdown());
		delete renderingSystem;

		assert(TF_OK == nativeContext->Shutdown());
		delete nativeContext;

		return TF_OK;
	}

}
