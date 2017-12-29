#pragma once

#include <Module.h>

// Temp Includes pending further investigation
#include <Entity.h>
#include <TransformComponent.h>
#include <CameraComponent.h>
#include <RenderingComponent.h>
#include <AnimationComponent.h>
#include <PhysicsComponent.h>

// Game: The main game loop.
// Runs all things game related.
class Game : public tofu::Module
{
public:
	virtual int32_t Init() override;

	virtual int32_t Shutdown() override;

	virtual int32_t Update() override;

	// These may be temporary or call the needed functions.
	// Will change int to file name later for JSON use.
	bool LoadScene(uint32_t);
	bool LoadOnTop(uint32_t);
	bool UnloadScene(uint32_t);

private:
	// Temp variables for testing
	tofu::TransformComponent tIntro;
	tofu::TransformComponent tCube;
	tofu::TransformComponent tBox;
	tofu::TransformComponent tBox2;
	tofu::TransformComponent tBox3;
	tofu::TransformComponent tBox4;
	tofu::TransformComponent tBox5;
	tofu::TransformComponent tBox6;
	tofu::TransformComponent tBox7;
	tofu::TransformComponent tGround;
	tofu::TransformComponent tPlayer;
	tofu::TransformComponent tCamera;
	tofu::PhysicsComponent pPlayer;
	tofu::AnimationComponent anim;

	// Class variables
	tofu::CameraComponent cam;

	enum sceneType
	{
		intro = 0,
		menu = 1,
		options = 2,
		help = 3,
		loading = 4,
		tutorial = 5,
		level = 6,
		pause = 7,
		death = 8,
		levelEnd = 9,
		credits = 10,
	};

	uint32_t currentScene;
	uint32_t lastScene;



	float pitch;
	float yaw;
	float speed;
	bool inAir;
};