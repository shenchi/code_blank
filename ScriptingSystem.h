#pragma once

#include "Script.h"

#include <unordered_map>
#include <string>
#include "common.h"

namespace tofu
{
	class ScriptingSystem
	{
		SINGLETON_DECL(ScriptingSystem)

	public:
		ScriptingSystem();

	public:
		int32_t Init();
		int32_t Shutdown();

		Script* LoadScript(const wchar_t* filename);

	private:
		void*	context;

		std::unordered_map<std::wstring, Script> scripts;
	};

}