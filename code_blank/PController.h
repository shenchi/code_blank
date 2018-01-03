#pragma once
#include "Camera.h"
#include "Player.h"


class PController
{
public:
	PController();
	~PController();

	void Update();
	void UpdateP(float);
	void SetCamera(Camera*);
	void SetPlayer(Player*);

	bool GetPause();

private:
	Camera* cam;
	Player* player;

	bool paused;
	bool inAir;
	float speed;

};