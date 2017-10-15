#pragma once

#include "common.h"

namespace tofu
{
	struct NativeContext;
	class Module;
	class RenderingSystem;
	class ScriptingSystem;

	class Engine
	{
		SINGLETON_DECL(Engine)

	public:
		Engine();

		int32_t Init(const wchar_t* filename);

		int32_t Run();

	private:

		int32_t Shutdown();

	private:
		NativeContext*		nativeContext;

		RenderingSystem*	renderingSystem;
		ScriptingSystem*	scriptingSystem;

		Module*				userModules[MAX_USER_MODULES];
		uint32_t			numUserModules;
	};
}