#include "Engine.h"

#include <cassert>

#include "NativeContext.h"

#include "ScriptingSystem.h"
#include "Script.h"

#include "RenderingSystem.h"

namespace tofu
{
	SINGLETON_IMPL(Engine);

	Engine::Engine()
	{
		assert(nullptr == _instance);
		_instance = this;
	}

	int32_t Engine::Init(const wchar_t* filename)
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

		renderingSystem = new RenderingSystem(RendererType::Auto);
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

		while(nativeContext->ProcessEvent())
		{

			scriptingSystem->Update();

			renderingSystem->Update();

		}

		err = Shutdown();
		return err;
	}

	int32_t Engine::Shutdown()
	{
		assert(TF_OK == nativeContext->Shutdown());
		delete nativeContext;

		assert(TF_OK == scriptingSystem->Shutdown());
		delete scriptingSystem;

		assert(TF_OK == renderingSystem->Shutdown());
		delete renderingSystem;


		return TF_OK;
	}

}
