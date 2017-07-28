#pragma once

#include "common.h"

namespace tofu
{
	class Script;

	struct NativeContext
	{
		virtual int32_t Init(Script* config) = 0;

		virtual int32_t Shutdown() = 0;

		virtual bool ProcessEvent() = 0;


		// 
		static NativeContext* Create();
	};



}
