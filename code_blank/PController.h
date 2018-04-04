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
	void FixedUpdate(float);
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
	bool jump;
	bool inAir;
	bool isAiming;
	bool isHacking;
	float speed;

	float pitch;
	float yaw;
	float camSenWGP = 2.0f;	// Cam sensitivity for gamepad
	float camSenWM = 0.5f;		// Cam sensitivity for mouse;

	int xAxisMod;
	int zAxisMod;

	int pitchMod;
	int yawMod;

	// Buttons
	bool attackButtonDown;
	bool dodgeButtonDown;
	bool specialButtonDown;
};