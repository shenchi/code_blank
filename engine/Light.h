#pragma once

#include "Common.h"
#include "TofuMath.h"

namespace tofu
{
	HANDLE_DECL(Light);

	enum LightType
	{
		kLightTypeDirectional,
		kLightTypePoint,
		kLightTypeSpot
	};

	class Light
	{
		friend class RenderingSystem;
	public:
		void SetType(LightType t) { type = t; }

		void SetColor(tofu::math::float4 t) { lightColor = t; }

		void CreateDepthMap() {}
	private:
		Light()
			:
			type(),
			handle(),
			lightColor(),
			depthMap()
		{}

	private:
		LightType	type;
		LightHandle	handle;
		tofu::math::float4	lightColor;
		TextureHandle  depthMap;
	};
}