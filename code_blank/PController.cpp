#include "PController.h"
#include <InputSystem.h>
#include <assert.h>
#include <PhysicsSystem.h>

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

	// Default control scheme: 1, 1, -1, 1
	// movement - horizontal, veritcal / camera - veritcal, horizontal
	SetControlMods(1, 1, -1, 1);
}

// Destructor
PController::~PController(){}

// Update, Non-Player dependant functions here
void PController::Update()
{
	if (input->IsButtonDown(ButtonId::kKeyEscape)) { paused = !paused; }

	if (input->IsGamepadConnected())
	{
		cam->SetSensitivity(camSenWGP);

		if (input->IsButtonDown(ButtonId::kGamepadStart))
		{
			paused = !paused;
		}
	}

}

// Fixed Update
void PController::FixedUpdate(float fDT)
{
	math::float3 inputDir = math::float3();

	// Gamepad Movement
	if (input->IsGamepadConnected())
	{
		inputDir.z = input->GetLeftStickY();
		inputDir.x = input->GetLeftStickX();
	}

	// Keyboard Movement
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

	// Dodge
	if ((input->IsButtonDown(kMouseRButton) || (input->IsButtonDown(kGamepadFaceRight)))
		&& !dodgeButtonDown)
	{
		player->Dodge(inputDir);
		dodgeButtonDown = true;
	}
	else if (!(input->IsButtonDown(kMouseRButton) || (input->IsButtonDown(kGamepadFaceRight)))
		&& dodgeButtonDown)
	{
		dodgeButtonDown = false;
	}

	if (!input->IsButtonDown(kKeyLeftShift)	// Stop Sprinting
		&& (input->GetRightTrigger() < 0.0001))
	{
		player->Sprint(false);
	}
	else if (input->IsButtonDown(kKeyLeftShift) // Sprint
		|| (input->GetRightTrigger() > 0))
	{
		player->Sprint(true);
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
		pitch = pitch * pitchMod * fDT;
		yaw = yaw * yawMod * fDT;

		cam->Rotate(math::float2{ pitch, yaw });

		cam->SetTarget(player->GetPosition());
	}
	else
	{
		cam->SetTarget(player->GetPosition());
	}

	// Final adjustment to movement based on any custom axis changes from player
	inputDir.x = inputDir.x * xAxisMod;
	inputDir.z = inputDir.z * zAxisMod;

	if (!isHacking)
	{
		player->MoveReg(fDT, jump, inputDir, cam->GetRotation());
	}
	else
	{
		// TODO
		// player hacking mode movement control
	}
}

// Update for Player 
void PController::UpdateP(float dT)
{
	assert(cam != NULL && player != NULL);
	

	// TODO
	// Add possible option to change which keys do what

	// Actions
	jump = input->IsButtonDown(ButtonId::kKeySpace)
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

	//// Dodge
	//if ( (input->IsButtonDown(kMouseRButton) || (input->IsButtonDown(kGamepadFaceRight)))
	//	&& !dodgeButtonDown )
	//{
	//	player->Dodge(inputDir);
	//	dodgeButtonDown = true;
	//}
	//else if (!(input->IsButtonDown(kMouseRButton) || (input->IsButtonDown(kGamepadFaceRight)))
	//	&& dodgeButtonDown)
	//{
	//	dodgeButtonDown = false;
	//}

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
}


//-------------------------------------------------------------------------------------------------
// Setters

// Set Active Camera
void PController::SetCamera(Camera* _cam)
{
	assert(_cam != NULL);
	cam = _cam;
	cam->SetSensitivity(camSenWM);
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