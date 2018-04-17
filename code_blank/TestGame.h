#pragma once

#include <Module.h>

#include <Entity.h>
#include <TransformComponent.h>
#include <CameraComponent.h>
#include <RenderingComponent.h>
#include <AnimationComponent.h>
#include <PhysicsComponent.h>
#include <LightComponent.h>
#include <AudioManager.h>

class TestGame : public tofu::Module
{
public:
	virtual int32_t Init() override;

	virtual int32_t Shutdown() override;

	virtual int32_t Update() override;

private:
	tofu::TransformComponent	tGround;
	tofu::TransformComponent	tBox;
	tofu::TransformComponent	tPlayer;
	tofu::TransformComponent	tCamera;
	tofu::TransformComponent    tSun;
	tofu::TransformComponent    tMoon;
	tofu::TransformComponent    tBulb;
	tofu::PhysicsComponent		pPlayer;
	tofu::AnimationComponent	anim;
	tofu::CameraComponent		cam;
	tofu::LightComponent        lSun;
	tofu::LightComponent        lMoon;
	tofu::LightComponent        lBulb;
	float pitch;
	float yaw;
	float speed;
	bool inAir;

	tofu::AudioSource			gameplay { "assets/sounds/Game_Play.wav" };
	tofu::AudioSource			jumpSfx { "assets/sounds/Jump.wav" };
};