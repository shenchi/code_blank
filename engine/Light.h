#pragma once

#include "Common.h"
#include "TofuMath.h"

namespace tofu
{
	HANDLE_DECL(Light);

	enum LightType
	{
		kLightTypeTest,
		kLightTypeDirectional,
		kLightTypePoint,
		kLightTypeSpot,
		kLightTypes
	};

	class Light
	{
		friend class RenderingSystem;
	public:

		void SetColor(tofu::math::float4 t) { lightColor = t; }

		void SetDirection(tofu::math::float3 t) { lightDirection = t; }

		void CreateDepthMap() {}
	private:
		Light(LightType type = LightType::kLightTypeTest)
			:
			type(type),
			handle(),
			lightColor(),
			lightDirection(),
			depthMap()
		{}

	private:
		LightType	type;
		LightHandle	handle;
		tofu::math::float4	lightColor;
		tofu::math::float3	lightDirection;
		TextureHandle  depthMap;
	};
}