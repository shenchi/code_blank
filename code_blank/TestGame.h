#pragma once

#include <Module.h>

#include <Entity.h>
#include <TransformComponent.h>
#include <CameraComponent.h>
#include <RenderingComponent.h>
#include <AnimationComponent.h>
#include <PhysicsComponent.h>

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
	tofu::PhysicsComponent		pPlayer;
	tofu::AnimationComponent	anim;
	tofu::CameraComponent		cam;
	float pitch;
	float yaw;
	float speed;
	bool inAir;
};
