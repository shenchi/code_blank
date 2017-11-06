#pragma once

#include "Common.h"

namespace tofu
{
	class Script;

	class NativeContext
	{
		SINGLETON_DECL(NativeContext)

	public:
		virtual int32_t Init(Script* config) = 0;

		virtual int32_t Shutdown() = 0;

		virtual bool ProcessEvent() = 0;

		virtual intptr_t GetContextHandle() = 0;

		virtual int64_t GetTimeCounter() = 0;

		virtual int64_t GetTimeCounterFrequency() = 0;

	public:
		static NativeContext* Create();
	};



}
