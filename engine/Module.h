#pragma once

#include "Common.h"

namespace tofu
{
	class Module
	{
	public:
		virtual int32_t Init() = 0;

		virtual int32_t Shutdown() = 0;

		virtual int32_t Update() = 0;

		virtual int32_t FixedUpdate() { return kOK; };

		virtual ~Module() {}
	};
}