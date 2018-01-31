#pragma once

#include <Tofu.h>
#include <LightComponent.h>

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
	// Dummy light
	tofu::TransformComponent    tSun;   
	tofu::LightComponent        lSun;
	float						pitch;
	float						yaw;
	float						speed;
};