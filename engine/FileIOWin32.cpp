#include "FileIO.h"

#include "MemoryAllocator.h"
#include <cstdio>
#include <cstdlib>

namespace tofu
{
	int32_t FileIO::ReadFile(const char* file, void** data, size_t* size, size_t alignment, uint32_t allocNo)
	{
		MemoryAllocator& alloc = MemoryAllocator::Allocators[allocNo];

		FILE* fp = nullptr;
		if (0 != fopen_s(&fp, file, "rb") || nullptr == fp)
		{
			return kErrUnknown;
		}

		// file size
		if (0 != fseek(fp, 0, SEEK_END))
		{
			fclose(fp);
			return kErrUnknown;
		}

		long fileSize = ftell(fp);
		if (fileSize < 0)
		{
			fclose(fp);
			return kErrUnknown;
		}

		if (0 != fseek(fp, 0, SEEK_SET))
		{
			fclose(fp);
			return kErrUnknown;
		}

		// allocate memory for content
		void* ptr = alloc.Allocate(fileSize, alignment);
		if (nullptr == ptr)
		{
			fclose(fp);
			return kErrUnknown;
		}

		if (1 != fread(ptr, fileSize, 1, fp))
		{
			// TODO we cannot deallocate here :(
			fclose(fp);
			return kErrUnknown;
		}

		fclose(fp);

		*data = ptr;
		*size = fileSize;

		return kOK;
	}
}
