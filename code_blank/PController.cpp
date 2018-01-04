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
	isAiming = false;
	isHacking = false;
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
	}

	// TODO
	// Add possible option to change which keys do what

	// Movement
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

	// Actions
	bool jump = input->IsButtonDown(ButtonId::kKeySpace)
		|| input->IsButtonDown(ButtonId::kGamepadFaceDown);

	if (input->IsButtonDown(kMouseLButton)	// Attack
		|| (input->IsButtonDown(kGamepadFaceLeft)) )
	{
		player->Attack();
	}

	if (input->IsButtonDown(kMouseRButton)	// Dodge
		|| (input->IsButtonDown(kGamepadFaceRight)) )
	{
		player->Dodge();
	}

	if (input->IsButtonDown(kKeyLeftControl) // Dash
		|| (input->GetRightTrigger() > 0) )
	{
		player->Dash();
	}

	if (input->IsButtonDown(kKeyE)	// Sword
		|| (input->IsButtonDown(kGamepadFaceUp)) )
	{
		player->Special();
	}

	if (input->IsButtonDown(kKeyQ)	// Vision Hack
		|| (input->IsButtonDown(kGamepadL1)) )
	{
		player->VisionHack();
	}

	if (input->IsButtonDown(kKeyF)	// Interact
		|| (input->IsButtonDown(kGamepadR1)))
	{
		player->Interact();
	}
	
	if (input->IsButtonDown(kKeyLeftShift)	// Aim Mode
		|| (input->GetLeftTrigger() > 0) )
	{
		player->Aim();
		isAiming = true;
	}
	else
	{
		isAiming = false;
	}

	// Change Camera control if in aiming mode
	if (!isAiming)
	{
		if (!input->IsGamepadConnected())
		{
			pitch = input->GetMouseDeltaY();
			yaw = input->GetMouseDeltaX();
		}
		else
		{
			// TODO
			// May need adjusting
			pitch = input->GetRightStickY();
			yaw = input->GetRightStickX();
		}

		cam->Rotate(math::float2{pitch, yaw});
		cam->UpdateTarget(player->GetPosition());
	}
	else
	{
		// TODO
		// Aiming mode camera control
		// If target is available (close enough and in right view angle) snap to
		// Possibly from a list of nearby enemies

		cam->UpdateTarget(player->GetPosition());
	}


	if (!isHacking && !isAiming)
	{
		player->MoveReg(dT, jump, inputDir, cam->GetRotation());
	}
	else if (isAiming)
	{
		player->MoveAim(dT, inputDir, cam->GetRotation(), cam->GetForward());
	}
	else
	{
		// TODO
		// player hacking mode movement control
	}
}

//-------------------------------------------------------------------------------------------------
// Setters

// Set Active Camera
void PController::SetCamera(Camera* _cam)
{
	assert(_cam != NULL);
	cam = _cam;
}

// Set Player Companion
void PController::SetCompanion(Companion* _comp)
{
	assert(_comp != NULL);
	comp = _comp;
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