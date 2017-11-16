#include "FileIO.h"

#include "MemoryAllocator.h"
#include <cstdio>
#include <cstdlib>

namespace tofu
{
	int32_t FileIO::ReadFile(const char* file, void** data, size_t* size, size_t alignment, uint32_t allocNo)
	{
		MemoryAllocator& alloc = MemoryAllocator::Allocators[allocNo];

		FILE* fp = fopen(file, "rb");
		if (nullptr == fp)
		{
			return TF_UNKNOWN_ERR;
		}

		// file size
		if (0 != fseek(fp, 0, SEEK_END))
		{
			fclose(fp);
			return TF_UNKNOWN_ERR;
		}

		long fileSize = ftell(fp);
		if (fileSize < 0)
		{
			fclose(fp);
			return TF_UNKNOWN_ERR;
		}

		if (0 != fseek(fp, 0, SEEK_SET))
		{
			fclose(fp);
			return TF_UNKNOWN_ERR;
		}

		// allocate memory for content
		void* ptr = alloc.Allocate(fileSize, alignment);
		if (nullptr == ptr)
		{
			fclose(fp);
			return TF_UNKNOWN_ERR;
		}

		if (1 != fread(ptr, fileSize, 1, fp))
		{
			// TODO we cannot deallocate here :(
			fclose(fp);
			return TF_UNKNOWN_ERR;
		}

		fclose(fp);

		*data = ptr;
		*size = fileSize;

		return TF_OK;
	}
}
