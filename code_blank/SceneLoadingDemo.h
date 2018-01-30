#pragma once

#include <Tofu.h>

class SceneLoadingDemo : public tofu::Module
{
public:
	virtual int32_t Init() override;

	virtual int32_t Shutdown() override;

	virtual int32_t Update() override;


private:
	tofu::SceneManager			sceneMgr;
	tofu::CameraComponent		cam;
	tofu::AnimationComponent	anim;
	tofu::TransformComponent	tPlayer;
	tofu::TransformComponent	tCamera;
	tofu::PhysicsComponent		pPlayer;
	float						pitch;
	float						yaw;
	float						speed;
};