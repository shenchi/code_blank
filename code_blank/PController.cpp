#include "PController.h"
#include <InputSystem.h>

using namespace tofu;

namespace
{
	InputSystem* input;
}

// Constructor
PController::PController()
{
	cam = NULL;
	player = NULL;
	input = InputSystem::instance();
	paused = false;
}

// Destructor
PController::~PController(){}

// Update, Non-Player dependant functions here
void PController::Update()
{
	if (input->IsButtonDown(ButtonId::kKeyEscape)) { paused = !paused; }

	if (input->IsGamepadConnected())
	{
		if (input->IsButtonDown(ButtonId::kGamepadFaceRight))
		{
			paused = !paused;
		}
	}

}

// Update for Player 
void PController::UpdateP(float dT)
{
	assert(cam != NULL && player != NULL);
	math::float3 inputDir = math::float3();

	if (input->IsGamepadConnected())
	{
		inputDir.z = -input->GetLeftStickY();
		inputDir.x = input->GetLeftStickX();

		//pitch += sensitive * input->GetRightStickY();
		//yaw += sensitive * input->GetRightStickX();
	}

	if (input->IsButtonDown(kKeyW))
	{
		inputDir.z = 1.0f;
	}
	else if (input->IsButtonDown(kKeyS))
	{
		inputDir.z = -1.0f;
	}

	if (input->IsButtonDown(kKeyD))
	{
		inputDir.x = 1.0f;
	}
	else if (input->IsButtonDown(kKeyA))
	{
		inputDir.x = -1.0f;
	}

	bool jump = input->IsButtonDown(ButtonId::kKeySpace)
		|| input->IsButtonDown(ButtonId::kGamepadFaceDown);

	cam->Rotate(math::float2{ input->GetMouseDeltaY(), input->GetMouseDeltaX() });
	cam->UpdateTarget(player->GetPosition());

	player->Act(dT, jump, inputDir, cam->GetRotation());
}

//-------------------------------------------------------------------------------------------------
// Setters

// Set Active Camera
void PController::SetCamera(Camera* _cam)
{
	assert(_cam != NULL);
	cam = _cam;
}

// Set Player
void PController::SetPlayer(Player* _player)
{
	assert(_player != NULL);
	player = _player;
}

//-------------------------------------------------------------------------------------------------
// Getters

// Should Game Pause
bool PController::GetPause()
{
	return paused;
}