#include "Transform.h"

namespace tofu
{
	using namespace math;

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
}

