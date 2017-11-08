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

		virtual bool ProcessEvent() = 0;

		virtual int32_t QuitApplication() = 0;

		virtual intptr_t GetContextHandle() = 0;

		virtual int64_t GetTimeCounter() = 0;

		virtual int64_t GetTimeCounterFrequency() = 0;

		virtual void UpdateInputStates(InputStates* states) = 0;

	public:
		static NativeContext* Create();
	};



}
