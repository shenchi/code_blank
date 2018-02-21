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

	attackButtonDown = false;
	dodgeButtonDown = false;
	specialButtonDown = false;

	// Default control scheme: 1, 1, 1, -1
	SetControlMods(1, 1, 1, -1);
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
		inputDir.z = input->GetLeftStickY();
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

	// Basic Attack
	if ( (input->IsButtonDown(kMouseLButton) || (input->IsButtonDown(kGamepadFaceLeft)) )
		&& !attackButtonDown )
	{
		player->Attack(true, dT);
		attackButtonDown = true;
	}
	else if( !(input->IsButtonDown(kMouseLButton) || (input->IsButtonDown(kGamepadFaceLeft))) 
		&&  attackButtonDown)
	{ 
		player->Attack(false, dT);
		attackButtonDown = false; 
	}

	// Dodge
	if ( (input->IsButtonDown(kMouseRButton) || (input->IsButtonDown(kGamepadFaceRight)))
		&& !dodgeButtonDown )
	{
		player->Dodge(inputDir);
		dodgeButtonDown = true;
	}
	else if (!(input->IsButtonDown(kMouseRButton) || (input->IsButtonDown(kGamepadFaceRight)))
		&& dodgeButtonDown)
	{
		dodgeButtonDown = false;
	}

	if(!input->IsButtonDown(kKeyLeftShift)	// Stop Sprinting
		&& (input->GetRightTrigger() < 0.0001) )
	{
		player->Sprint(false);
	}
	else if (input->IsButtonDown(kKeyLeftShift) // Sprint
		|| (input->GetRightTrigger() > 0))
	{
		player->Sprint(true);
	}


	// Sword Attack
	if ((input->IsButtonDown(kKeyE) || (input->IsButtonDown(kGamepadFaceUp)))
		&& !specialButtonDown)
	{
		player->Special(true, dT);
		specialButtonDown = true;
	}
	else if (!(input->IsButtonDown(kKeyE) || (input->IsButtonDown(kGamepadFaceUp)))
		&& specialButtonDown)
	{
		player->Special(false, dT);
		specialButtonDown = false;
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
	
	// Aim Mode
	if ( (input->IsButtonDown(kKeyLeftShift) || (input->GetLeftTrigger() > 0) )
		&& !isAiming )
	{
		player->Aim(true);
		isAiming = true;
		
	}
	else if(isAiming)
	{
		player->Aim(false);
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

		// Camera control modification based on player settings
		pitch = pitch * pitchMod;
		yaw = yaw * yawMod;

		cam->Rotate(math::float2{pitch, yaw});
		cam->UpdateTarget(player->GetPosition());
	}
	else
	{
		cam->UpdateTarget(player->GetPosition());
	}

	// Final adjustment to movement based on any custom axis changes from player
	inputDir.x = inputDir.x * xAxisMod;
	inputDir.z = inputDir.z * zAxisMod;

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

// Set the custom player axis controls
// player left/right, player forward/backward, cam up/down, cam left/right
void PController::SetControlMods(int _xAxisMod, int _zAxisMod, int _pitchMod, int _yawMod)
{
	xAxisMod = _xAxisMod;
	zAxisMod = _zAxisMod;
	pitchMod = _pitchMod;
	yawMod = _yawMod;
}

//-------------------------------------------------------------------------------------------------
// Getters

// Should Game Pause
bool PController::GetPause()
{
	return paused;
}