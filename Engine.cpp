#include "Engine.h"

#include <cassert>

#include "NativeContext.h"

#include "ScriptingSystem.h"
#include "Script.h"

#include "RenderingSystem.h"

namespace tofu
{
	float Time::TotalTime = 0.0f;
	float Time::DeltaTime = 0.0f;

	SINGLETON_IMPL(Engine);

	Engine::Engine()
		:
		nativeContext(nullptr),
		renderingSystem(nullptr),
		scriptingSystem(nullptr),
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

		// read config

		Script s(filename);
		if (!s) 
			return TF_CONFIG_LOADING_FAILED;

		// initialize native context

		nativeContext = NativeContext::Create();
		if (nullptr == nativeContext)
			return TF_UNKNOWN_ERR;

		err = nativeContext->Init(&s);
		if (TF_OK != err)
			return err;

		// initialize rendering system

		renderingSystem = new RenderingSystem();
		err = renderingSystem->Init();
		if (TF_OK != err)
			return err;

		// initialize scripting system

		scriptingSystem = new ScriptingSystem();
		err = scriptingSystem->Init();
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

		while(nativeContext->ProcessEvent())
		{
			lastTimeCounter = currentTimeCounter;
			currentTimeCounter = nativeContext->GetTimeCounter();

			int64_t deltaTimeCounter = (currentTimeCounter - lastTimeCounter) * 1000000;

			Time::DeltaTime = (deltaTimeCounter / timeCounterFreq) / 1000000.0f;
			Time::TotalTime += Time::DeltaTime;

			renderingSystem->BeginFrame();

			scriptingSystem->Update();

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

	int32_t Engine::Shutdown()
	{
		for (uint32_t i = 0; i < numUserModules; i++)
		{
			assert(TF_OK == userModules[i]->Shutdown());
			delete userModules[i];
		}

		assert(TF_OK == nativeContext->Shutdown());
		delete nativeContext;

		assert(TF_OK == scriptingSystem->Shutdown());
		delete scriptingSystem;

		assert(TF_OK == renderingSystem->Shutdown());
		delete renderingSystem;

		return TF_OK;
	}

}
