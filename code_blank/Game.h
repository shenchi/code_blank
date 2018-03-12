#pragma once

#include <Tofu.h>
#include "Character.h"
#include "PController.h"
#include "Camera.h"
#include "Enemy.h"

// Game: The main game loop.
// Runs all things game related.
class Game : public tofu::Module
{
public:
	virtual int32_t Init() override;

	virtual int32_t Shutdown() override;

	virtual int32_t Update() override;

	virtual ~Game() override;

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

	tofu::TransformComponent tStair;

	// Class variables
	//tofu::CameraComponent cam;
	Camera* cam;
	Player* player;
	Companion* comp;
	PController* pControl;

	std::vector<Character*>* enemyList;

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

	tofu::SceneManager sceneMgr;
	sceneType currentScene;
	sceneType lastScene;

	// Member Functions
	// These may be temporary or call the needed functions.
	// Will change int to file name later for JSON use.
	bool LoadScene(sceneType);
	bool UnloadScene(sceneType);
	bool LoadOnTop(sceneType);
	bool UnloadOffTop(sceneType);

	//*********************************************************************************************
	//temp for test
	//Enemy* enemy01;
	//Enemy* enemy02;
	//Enemy* enemy03;
	float timePassed;
	float startTime;
	bool loopStart;
	//*********************************************************************************************
};