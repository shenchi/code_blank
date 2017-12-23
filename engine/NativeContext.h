#pragma once

#include "Common.h"

namespace tofu
{
	struct InputStates;

	class NativeContext
	{
		SINGLETON_DECL(NativeContext)

	public:
		virtual ~NativeContext() {}

		virtual int32_t Init() = 0;

		virtual int32_t Shutdown() = 0;

		// platform native event handling
		// !!! return false to indicate engine to quit main loop
		virtual bool ProcessEvent() = 0;

		// post a request to quit and application
		virtual int32_t QuitApplication() = 0;

		// rendering context( Win32: HWND )
		virtual intptr_t GetContextHandle() = 0;

		// system performance counter (used for engine timing)
		virtual int64_t GetTimeCounter() = 0;

		// frequenct of performane counter
		virtual int64_t GetTimeCounterFrequency() = 0;

		// update input states from platform (only call this once per frame)
		virtual void UpdateInputStates(InputStates* states) = 0;

	public:

		//
		static NativeContext* Create();
	};



}
