#pragma once

#include "Common.h"

namespace tofu
{
	class FileIO
	{
	public:
		static int32_t ReadFile(const char* file, void** data, size_t* size, size_t alignment, uint32_t allocNo);
	};
}
