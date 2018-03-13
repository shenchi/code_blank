#pragma once

#include <Tofu.h>

class RenderingTest : public tofu::Module
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
	tofu::TransformComponent	tBox;
	// Dummy light
	tofu::TransformComponent    tSun;
	tofu::LightComponent        lSun;
	tofu::TransformComponent    tMoon;
	tofu::TransformComponent    tBulb;
	tofu::LightComponent        lMoon;
	tofu::LightComponent        lBulb;
	float						pitch;
	float						yaw;
	float						speed;
};