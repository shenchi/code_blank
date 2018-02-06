#pragma once

#include <Tofu.h>

namespace Utility
{
	class GhostPlayer
	{
	public:
		GhostPlayer(tofu::math::float3 pos, tofu::Material* skybox = nullptr);
		~GhostPlayer();

		int32_t Update();

	private:
		tofu::Entity				entity;
		tofu::TransformComponent	transform;
		tofu::CameraComponent		camera;
		float						pitch;
		float						yaw;
		float						speed;
	};
}