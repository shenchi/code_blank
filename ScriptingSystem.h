#pragma once

#include "Common.h"

#include "Script.h"

#include "Module.h"

#include <unordered_map>
#include <string>

namespace tofu
{
	class ScriptingSystem : public Module
	{
		SINGLETON_DECL(ScriptingSystem)

	public:
		ScriptingSystem();

	public:
		int32_t Init() override;

		int32_t Shutdown() override;

		int32_t Update() override;

		Script* LoadScript(const char* filename);

	private:
		void*	context;

		std::unordered_map<std::string, Script> scripts;
	};

}