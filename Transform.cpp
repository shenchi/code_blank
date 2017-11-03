#include "Transform.h"

namespace tofu
{
	using namespace math;

	Transform Transform::operator*(const Transform& other) const
	{
		Transform t;
		
		return t;
	}

	float4x4 Transform::GetMatrix() const
	{
		return math::float4x4();
	}
}

