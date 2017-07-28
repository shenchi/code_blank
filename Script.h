#pragma once

#include <string>

namespace tofu
{
	class ScriptingSystem;

	class Script
	{
	private:
		friend class ScriptingSystem;
		Script(void* context, const wchar_t* filename);

	public:
		Script(const wchar_t* filename);
		~Script();

		Script(const Script& s) = delete;
		Script& operator = (const Script& s) = delete;

		Script(Script&& s);
		Script& operator = (Script&& s);

		inline operator bool() const {	return (nullptr != context); }

	public:
		bool			Exist(char* variable);

		bool			GetBool(char* variable, bool defVal = false);
		int32_t			GetInt32(char* variable, int32_t defVal = 0);
		int64_t			GetInt64(char* variable, int64_t defVal = 0l);
		float			GetFloat(char* variable, float defVal = 0.0f);
		std::string		GetString(char* variable, const std::string& defVal = "");

	private:

		void			LoadFromFile(const wchar_t* filename);

	private:
		void*	context;
		bool	standalone;

	private:
		int32_t LoadVariable(char* name);
	};
}
