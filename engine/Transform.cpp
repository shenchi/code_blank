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
		t.translation = other.rotation * (other.scale * translation) + other.translation;

		return t;
	}

	Transform Transform::operator*(const float multiplier) const
	{
		return Transform(translation * multiplier, rotation * multiplier, scale * multiplier);
	}

	Transform& Transform::operator*=(const float multiplier)
	{
		translation *= multiplier;
		rotation = normalize(rotation * multiplier);
		scale *= multiplier;

		return *this;
	}

	float4x4 Transform::GetMatrix() const
	{
		return transform(translation, rotation, scale);
	}

	float3 Transform::TransformVector(const float3 & v) const
	{
		return rotation * (v * scale);
	}
	
	float4 Transform::TransformVector(const float4 & v) const
	{
		float3 ret = TransformVector(float3{ v.x, v.y, v.z });
		return float4{ ret.x, ret.y, ret.z, v.w };
	}
	
	float3 Transform::TransformPosition(const float3 & v) const
	{
		return rotation * (v * scale) + translation;
	}

	float4 Transform::TransformPosition(const float4 & v) const
	{
		float3 ret = TransformPosition(float3{ v.x, v.y, v.z });
		return float4{ ret.x, ret.y, ret.z, v.w };
	}

	void Transform::SetToRelativeTransform(const Transform& parent) {
		const quat inverseRot = inverse(parent.rotation);

		// assert scale != 0
		scale /= parent.scale;
		translation = (inverseRot * (translation - parent.translation)) / parent.scale;
		rotation = inverseRot * rotation;
	}

	void Transform::Blend(const Transform other, float weight)
	{
		translation = mix(translation, other.translation, weight);
		rotation = slerp(rotation, other.rotation, weight);
		scale = mix(scale, other.scale, weight);

		//normalize(rotation);
	}

	void Transform::Additive(const Transform other, float weight)
	{
		translation += other.translation * weight;
		scale += other.scale * weight;

		quat temp = slerp(quat(), other.rotation, weight);

		if (dot(rotation, temp) < 0.f) {
			temp = -temp;
		}

		rotation = temp * rotation;
		//rotation = normalize(temp * rotation);
	}
}
