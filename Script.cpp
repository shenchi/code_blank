#include "Script.h"

#include <lua.hpp>
#include <fstream>

namespace tofu
{

	Script::Script(void* context, const wchar_t* filename)
		:
		context(context), standalone(false)
	{
		LoadFromFile(filename);
	}

	Script::Script(const wchar_t * filename)
		:
		context(nullptr), standalone(true)
	{
		context = luaL_newstate();
		if (nullptr != context)
			LoadFromFile(filename);
	}

	Script::~Script()
	{
		if (standalone && nullptr != context)
		{
			lua_close(reinterpret_cast<lua_State*>(context));
		}
	}

	Script::Script(Script && s)
		:
		context(s.context), standalone(s.standalone)
	{
		s.context = nullptr;
		// TODO other clean up
	}

	Script & Script::operator=(Script && s)
	{
		if (this != &s)
		{
			context = s.context;
			standalone = s.standalone;

			s.context = nullptr;
			// TODO other clean up
		}
		return *this;
	}

	bool Script::Exist(char * variable)
	{
		if (nullptr == context) return false;
		auto* L = reinterpret_cast<lua_State*>(context);

		int32_t nElem = LoadVariable(variable);
		bool ret = nElem > 0;
		lua_pop(L, nElem);
		return ret;
	}

	bool Script::GetBool(char * variable, bool defVal)
	{
		if (nullptr == context) return false;
		auto* L = reinterpret_cast<lua_State*>(context);

		bool ret = defVal;

		int32_t nElem = LoadVariable(variable);
		if (nElem > 0)
		{
			ret = (lua_toboolean(L, -1) != 0);
		}
		lua_pop(L, nElem);
		return ret;
	}

	int32_t Script::GetInt32(char * variable, int32_t defVal)
	{
		if (nullptr == context) return false;
		auto* L = reinterpret_cast<lua_State*>(context);

		int32_t ret = defVal;

		int32_t nElem = LoadVariable(variable);
		if (nElem > 0)
		{
			ret = static_cast<int32_t>(lua_tointeger(L, -1));
		}
		lua_pop(L, nElem);
		return ret;
	}

	int64_t Script::GetInt64(char * variable, int64_t defVal)
	{
		if (nullptr == context) return false;
		auto* L = reinterpret_cast<lua_State*>(context);

		int64_t ret = defVal;

		int32_t nElem = LoadVariable(variable);
		if (nElem > 0)
		{
			ret = lua_tointeger(L, -1);
		}
		lua_pop(L, nElem);
		return ret;
	}

	float Script::GetFloat(char * variable, float defVal)
	{
		if (nullptr == context) return false;
		auto* L = reinterpret_cast<lua_State*>(context);

		float ret = defVal;

		int32_t nElem = LoadVariable(variable);
		if (nElem > 0)
		{
			ret = static_cast<float>(lua_tonumber(L, -1));
		}
		lua_pop(L, nElem);
		return ret;
	}

	std::string Script::GetString(char * variable, const std::string& defVal)
	{
		if (nullptr == context) return false;
		auto* L = reinterpret_cast<lua_State*>(context);

		std::string ret = defVal;

		int32_t nElem = LoadVariable(variable);
		if (nElem > 0)
		{
			ret = lua_tostring(L, -1);
		}
		lua_pop(L, nElem);
		return ret;
	}

	void Script::LoadFromFile(const wchar_t * filename)
	{
		std::ifstream fin(filename, std::ios::in | std::ios::binary);
		if (!fin.is_open())
			return;

		char* buf = nullptr;
		lua_State* L = reinterpret_cast<lua_State*>(context);

		do
		{
			size_t length = fin.seekg(0, std::ios::_Seekend).tellg();
			fin.seekg(0, std::ios::_Seekbeg);

			buf = new char[length];
			if (!fin.read(buf, length))
				break;

			if (LUA_OK != luaL_loadbuffer(L, buf, length, nullptr))
				break;

			if (LUA_OK != lua_pcall(L, 0, 0, 0))
				break;

		} while (0);

		if (nullptr != buf) delete[] buf;
		fin.close();
	}

	int32_t Script::LoadVariable(char * name)
	{
		if (nullptr == context) return -1;
		auto* L = reinterpret_cast<lua_State*>(context);

		std::string namePath(name);
		size_t start = 0, end = std::string::npos;

		int level = 0;

		do
		{
			end = namePath.find('.', start);
			std::string vname = namePath.substr(start, end);
			start = end + 1;

			if (level == 0)
			{
				if (LUA_TNIL == lua_getglobal(L, vname.c_str()))
					return 0;
			}
			else
			{
				if (!lua_istable(L, -1) || LUA_TNIL == lua_getfield(L, -1, vname.c_str()))
				{
					lua_pop(L, level);
					return 0;
				}
			}

			level++;
		} while (std::string::npos != end);

		return level;
	}

}
