#include "CameraComponent.h"
#include "TransformComponent.h"

namespace tofu
{
	using namespace math;

	void CameraComponentData::LookAt(const float3& target)
	{
		TransformComponent t = entity.GetComponent<TransformComponent>();
		assert(true == t);
		// TODO
	}

	void CameraComponentData::LookTo(const float3& direction)
	{
		TransformComponent t = entity.GetComponent<TransformComponent>();
		assert(true == t);
	}

	math::float4x4 CameraComponentData::CalcViewMatrix() const
	{
		TransformComponent t = entity.GetComponent<TransformComponent>();
		assert(true == t);

		return math::float4x4();
	}

	math::float4x4 CameraComponentData::CalcProjectionMatrix() const
	{
		return math::float4x4();
	}


}

