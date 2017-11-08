#include "Transform.h"

namespace tofu
{
	using namespace math;

	// a * b  - apply transform a and then transform b
	Transform Transform::operator*(const Transform& other) const
	{
		Transform t;

		t.rotation = other.rotation * rotation;
		t.scale = scale * other.scale;
		t.translation = other.rotation.rotate(other.scale * translation) + other.translation;

		return t;
	}

	float4x4 Transform::GetMatrix() const
	{
		return matrix::transform(translation, rotation, scale);
	}

	float3 Transform::TransformVector(const float3 & v) const
	{
		float3 ret = rotation.rotate(v * scale);

		return normalize(ret);
	}
	
	float4 Transform::TransformVector(const float4 & v) const
	{
		float3 ret = TransformVector(float3{ v.x, v.y, v.z });
		return float4{ ret.x, ret.y, ret.z, v.w };
	}
	
	float3 Transform::TransformPosition(const float3 & v) const
	{
		return rotation.rotate(v * scale) + translation;
	}

	float4 Transform::TransformPosition(const float4 & v) const
	{
		float3 ret = TransformPosition(float3{ v.x, v.y, v.z });
		return float4{ ret.x, ret.y, ret.z, v.w };
	}
}

