#pragma once

#include "common.h"

namespace tofu
{
	enum class RendererType
	{
		Auto = 0,
		Direct3D11,
	};

	struct Renderer
	{
		virtual int32_t Init() = 0;
		virtual int32_t Release() = 0;





		static Renderer* CreateRenderer(RendererType type);
	};

}

