#pragma once

#include "Common.h"
#include "Component.h"
#include "TofuMath.h"

namespace tofu
{
	enum LightType
	{
		kLightTypeDirectional,
		kLightTypePoint,
		kLightTypeSpot
	};

	class LightComponentData
	{
		friend class RenderingSystem;
	public:
		LightComponentData() : LightComponentData(Entity()) {}
		LightComponentData(Entity e)
			:
			entity(e),
			type(kLightTypeDirectional),
			lightColor(),
			depthMap()
		{}
		
		void SetType(LightType t) { type = t; }

		void SetColor(tofu::math::float4 t) { lightColor = t; }

		void CreateDepthMap() {}

	private:
		Entity entity;
		LightType	type;
		tofu::math::float4	lightColor;

		TextureHandle  depthMap;
	};

	typedef Component<LightComponentData> LightComponent;
}