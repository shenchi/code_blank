#pragma once
#include <Tofu.h>
#include <LightComponent.h>

class AnimationDemo : public tofu::Module
{
public:
	virtual int32_t Init() override;

	virtual int32_t Shutdown() override;

	virtual int32_t Update() override;

private:
	tofu::Entity				eBox;

	tofu::CameraComponent		cam;
	tofu::AnimationComponent	anim;
	tofu::TransformComponent	tPlayer;
	tofu::TransformComponent	tPlayer2;
	tofu::TransformComponent	tBox;
	tofu::TransformComponent	tCamera;

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

