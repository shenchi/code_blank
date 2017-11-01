#pragma once

#include "Common.h"
#include "Component.h"
#include "TofuMath.h"

namespace tofu
{
	enum class ProjectionType : uint32_t
	{
		Perspective,
		Orthographic
	};

	class CameraComponentData
	{
	public:
		CameraComponentData() : CameraComponentData(Entity()) {}

		CameraComponentData(Entity e) : entity(e) {}

		void LookAt(const math::float3& target);

		void LookTo(const math::float3& direction);

		void SetProjectionType(ProjectionType type) { projType = type; }

		ProjectionType GetProjectionType() const { return projType; }

		math::float4x4 CalcViewMatrix() const;

		math::float4x4 CalcProjectionMatrix() const;

	private:
		Entity					entity;
		ProjectionType			projType;
		float					aspect;
		float					zNear;
		float					zFar;
	};

	typedef Component<CameraComponentData> CameraComponent;
}