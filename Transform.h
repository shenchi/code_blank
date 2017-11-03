#pragma once

#include "TofuMath.h"

namespace tofu
{
	class Transform
	{
	public:
		Transform()
			:
			rotation(),
			translation(),
			scale()
		{}

		inline const math::float3&	GetTranslation() const { return translation; }

		inline const math::float4&	GetRotation() const { return rotation; }

		inline const math::float3&	GetScale() const { return scale; }


		inline void					SetTranslation(const math::float3& t) { translation = t; }

		inline void					SetTranslation(float x, float y, float z) { translation = { x, y, z }; }


		inline void					SetRotation(const math::float4& q) { rotation = q; }


		inline void					SetScale(const math::float3& s) { scale = s; }

		inline void					SetScale(float s) { scale = { s, s, s }; }

		inline void					SetScale(float x, float y, float z) { scale = { x, y, z }; }


		Transform operator * (const Transform&) const;

		math::float4x4				GetMatrix() const;

	private:
		math::float4				rotation;
		math::float3				translation;
		math::float3				scale;
	};
}
