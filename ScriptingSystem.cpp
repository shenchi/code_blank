#include "ScriptingSystem.h"

#include "Script.h"

#include <lua.hpp>
#include <cassert>
#include "StringUtils.h"

namespace tofu
{
	SINGLETON_IMPL(ScriptingSystem);

	ScriptingSystem::ScriptingSystem()
	{
		assert(nullptr == _instance);
		_instance = this;
	}

	int32_t ScriptingSystem::Init()
	{
		context = luaL_newstate();

		if (nullptr == context)
			return TF_UNKNOWN_ERR;

		scripts.clear();

		return TF_OK;
	}

	int32_t ScriptingSystem::Shutdown()
	{
		return TF_OK;
	}

	int32_t ScriptingSystem::Update()
	{
		return int32_t();
	}

	Script* ScriptingSystem::LoadScript(const char* filename)
	{
		std::string path(filename);
		std::string basename = StringUtils::Basename(path);

		if (scripts.find(basename) != scripts.end())
			return &(scripts.at(basename));

		scripts.insert(std::pair<std::wstring, Script>(basename, Script(context, filename)));

		return &(scripts.at(basename));
	}
}

