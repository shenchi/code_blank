#pragma once

#include "Common.h"
#include "Component.h"
#include "TofuMath.h"

namespace tofu
{
	class Material;
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
			range(1.0f),
			intensity(1.0f),
			spotAngle(45.0f),
			castShadow(false)
		{}
		
		void SetType(LightType t) { type = t; }

		void SetColor(const math::float4& t) { lightColor = t; }

		void SetRange(float r) { range = r; }

		void SetIntensity(float i) { intensity = i; }

		void SetSpotAngle(float angle) { spotAngle = angle; }

		void SetCastShadow(bool s) { castShadow = s; }

	private:
		Entity				entity;
		LightType			type;
		math::float4		lightColor;
		float				range;
		float				intensity;
		float				spotAngle;
		bool				castShadow;
	};

	typedef Component<LightComponentData> LightComponent;
}