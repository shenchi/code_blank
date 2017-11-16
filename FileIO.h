#pragma once

#include "Common.h"

namespace tofu
{
	class FileIO
	{
	public:
		// read a file to a new allocated memory from allocator[allocNo]
		static int32_t ReadFile(const char* file, void** data, size_t* size, size_t alignment, uint32_t allocNo);
	};
}
