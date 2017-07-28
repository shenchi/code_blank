#pragma once

#include <string>

namespace tofu
{
	struct StringUtils
	{
		inline static std::wstring Widen(const std::string& str)
		{
			return std::wstring(str.begin(), str.end());
		}

		inline static std::wstring Basename(const std::wstring& str)
		{
			size_t slash = str.find_last_of(L'/');
			size_t backslash = str.find_last_of(L'\\');
			size_t dot = str.find_last_of(L'.');
			size_t start = 0, count = std::wstring::npos;

			if (slash != std::wstring::npos && slash > start) start = slash;
			if (backslash != std::wstring::npos && backslash > start) start = backslash;
			if (dot != std::wstring::npos && dot >= start) count = dot - start;

			return str.substr(start, count);
		}
	};
}

