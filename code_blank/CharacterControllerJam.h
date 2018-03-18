#pragma once

#include <Tofu.h>

enum CharacterStates
{
	kStateIdle,
};

class CharacterControllerJam : public tofu::Module
{
public:
	virtual int32_t Init() override;

	virtual int32_t Shutdown() override;

	virtual int32_t Update() override;

private:

	void AddAnimState(const char* name, const char* clipname = nullptr);

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

	CharacterStates				state;
};