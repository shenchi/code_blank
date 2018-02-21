#pragma once
#include "Camera.h"
#include "Player.h"
#include "Companion.h"

class PController
{
public:
	PController();
	~PController();

	void Update();
	void UpdateP(float);
	void SetCamera(Camera*);
	void SetCompanion(Companion*);
	void SetPlayer(Player*);
	void SetControlMods(int, int, int, int);

	bool GetPause();

private:
	Camera* cam;
	Player* player;
	Companion* comp;

	bool paused;
	bool inAir;
	bool isAiming;
	bool isHacking;
	float speed;

	float pitch;
	float yaw;

	int xAxisMod;
	int zAxisMod;

	int pitchMod;
	int yawMod;

	// Buttons
	bool attackButtonDown;
	bool dodgeButtonDown;
	bool specialButtonDown;
};