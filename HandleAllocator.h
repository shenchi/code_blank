#pragma once

#include "Common.h"
#include <cassert>

namespace tofu
{
	template<class Handle, uint32_t Count>
	class HandleAllocator
	{
	public:
		inline HandleAllocator()
			:
			numInUse(0)
		{
			for (uint32_t i = 0; i < Count; ++i)
			{
				handles[i] = UINT32_MAX;
				indices[i] = i;
			}
		}

		inline Handle Allocate()
		{
			if (numInUse < Count)
			{
				uint32_t idx = numInUse;
				uint32_t id = indices[idx];

				assert(handles[id] == UINT32_MAX);

				handles[id] = idx;
				numInUse++;

				return Handle(id);
			}
			return Handle();
		}

		inline void Free(Handle handle)
		{
			assert(true == handle);

			uint32_t idx = handles[handle.id];

			assert(idx < numInUse);

			uint32_t lastIdx = numInUse - 1;
			if (idx != lastIdx)
			{
				uint32_t lastHandle = indices[lastIdx];

				indices[lastIdx] = handle.id;

				indices[idx] = lastHandle;
				handles[lasthandle] = idx;
			}

			handles[handle.id] = UINT32_MAX;

			numInUse--;
		}

	private:
		uint32_t		handles[Count];
		uint32_t		indices[Count];
		uint32_t		numInUse;
	};
}