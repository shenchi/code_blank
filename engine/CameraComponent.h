#pragma once

#include "Common.h"
#include "Component.h"
#include "TofuMath.h"

namespace tofu
{
	class Material;

	enum class ProjectionType : uint32_t
	{
		kProjectionTypePerspective,
		kProjectionTypeOrthographic
	};

	class CameraComponentData
	{
		friend class RenderingSystem;
	public:
		CameraComponentData() : CameraComponentData(Entity()) {}

		CameraComponentData(Entity e)
			:
			entity(e),
			projType(ProjectionType::kProjectionTypePerspective),
			fov(90.0f),
			aspect(1.0f),
			zNear(0.01f),
			zFar(100.0f),
			clearColor{ 0.0f, 0.0f, 0.0f, 1.0f },
			skybox(nullptr)
		{}

		void LookAt(const math::float3& target);

		void LookTo(const math::float3& direction);


		void SetProjectionType(ProjectionType type) { projType = type; }

		ProjectionType GetProjectionType() const { return projType; }


		inline void SetPerspective(float fov, float aspect, float zNear, float zFar)
		{
			this->fov = fov;
			this->aspect = aspect;
			this->zNear = zNear;
			this->zFar = zFar;
		}


		// set fov in degree
		inline void SetFOV(float fov) { this->fov = fov; }

		// get fov in degree
		inline float GetFOV() const { return fov; }


		inline void SetAspect(float aspect) { this->aspect = aspect; }

		inline float GetAspect() const { return aspect; }


		inline void SetZNear(float z) { zNear = z; }

		inline float GetZNear() const { return zNear; }


		inline void SetZFar(float z) { zFar = z; }

		inline float GetZFar() const { return zFar; }


		inline void SetClearColor(const math::float4& color) { clearColor = color; }

		inline const math::float4& GetClearColor() const { return clearColor; }


		inline void SetSkybox(Material* mat) { skybox = mat; }

		inline Material* GetSkybox() const { return skybox; }


		math::float4x4 CalcViewMatrix() const;

		math::float4x4 CalcProjectionMatrix() const;

	private:
		Entity					entity;
		ProjectionType			projType;
		float					fov;			// in degree
		float					aspect;
		float					zNear;
		float					zFar;

		math::float4			clearColor;
		Material*				skybox;
	};

	typedef Component<CameraComponentData> CameraComponent;
}