#include "Character.h"
#include <PhysicsSystem.h>

using namespace tofu;


Character::Character()
{}

Character::~Character()
{
	delete combatManager;
	delete gCharacter;
}

void Character::Init(bool isPlayer, void* comp, CombatManagerDetails combatDetails)
{
	combatManager = new CombatManager(isPlayer, comp, this, combatDetails);
	gCharacter = new GameplayAnimationMachine(combatManager);
	physics = tofu::PhysicsSystem::instance();
	jumpTimer = 0.0f;
	jumpDelay = 0.016f;
	queueJump = false;
	once = true;
}


void Character::Update(float dT)
{
	if (once)
	{
		once = false;
		pCharacter->LockRotation(true, true, true);
	}
	combatManager->Update(dT);
	if (jumpTimer > 0)
	{
		if (dT < 0.0159f)
		{
			jumpTimer -= 0.016f;
		}
		else
		{
			jumpTimer -= dT;
		}
	}

	if (queueJump)
	{
		if (dT < 0.0159f)
		{
			jumpDelay -= 0.016f;
		}
		else
		{
			jumpDelay -= dT;
		}
	}
	if(queueJump && jumpDelay < 0)
	{
		// Original Values 0.0f, 4.0f, 0.0f
		pCharacter->ApplyImpulse(math::float3{ 0.0f, 5.0f, 0.0f });
				
		queueJump = false;

		isGrounded = false;		
	}
}

void Character::UpdateState(float dT)
{}




// Handle movement on the ground
void Character::HandleGroundedMovement(bool _jump, math::float3 move, float dT)
{
	// check whether conditions are right to allow a jump
	if (_jump && !jump && isGrounded && jumpTimer < 0.001f)
	{
		jumpTimer = 0.5f;
		//charAudio.Stop();
		//charAudio.PlayOneShot(jumpFX);

		// jump!
		//rigidbody.velocity = math::float3(m_Rigidbody.velocity.x, jumpPower, m_Rigidbody.velocity.z);

		// TODO
		// Current jump mechanic
		// need to wait a frame before jumping
		//pCharacter->ApplyImpulse(math::float3{ 0.0f, 4.0f, 0.0f });
		queueJump = true;
		jumpDelay = 0.768f;

		combatManager->SetIsJumping(true);
		jump = true;
		stateTimer = 0;

		//move.y = jumpPower * dT * move.y;

		//tCharacter->Translate(math::float3{ move.x, 1.0f, move.z });
		//isGrounded = false;

		//jump state
		//combatManager->SetIsJumping(true);
		//jump = true;
		//stateTimer = 0;
	}
	
	tCharacter->Translate(move);
	//math::float3 temp = { 0,0,0 };
	//pCharacter->LockRotation(false, false, false);
	//pCharacter->LockPosition(false, false, false);
	//temp.x = move.x * 20.0f;
	//temp.y = 0.0f;
	//temp.z = move.z * 20.0f;
	//pCharacter->SetVelocity(temp);
	
} // end ground movement

 // TODO
 // Needs fixing in the Unity version and then updated here
 // Handle airborne movement
 
void Character::HandleAirborneMovement(math::float3 move, math::float3 inputDir, float dT)
{
	//float y = tCharacter->GetWorldPosition().y;

	float y = move.y;

	//move += 10.0f * dT * move;
	if (move.x == 0)
	{
		move.x = dT * inputDir.x * 2.0f;
	}

	if (move.z == 0)
	{
		move.z = dT * inputDir.z * 2.0f;
	}
	move.y = y;

	tCharacter->Translate(move);

	// apply extra gravity from multiplier:
	//math::float3 extraGravityForce = (Physics.gravity * gravityMultiplier) - Physics.gravity;
	//rigidbody.AddForce(extraGravityForce);
	//rigidbody.velocity = math::float3(rigidbody.velocity.x - (inputDir.z / 10), rigidbody.velocity.y, rigidbody.velocity.z + (inputDir.x / 10));

	//groundCheckDistance = m_Rigidbody.velocity.y < 0 ? origGroundCheckDistance : 0.01f;
	
	/*
	math::float3 vel = pCharacter->GetVelocity();

	tofu::math::float3 move;
	if (inputDir.length > 0)
	{
		move = inputDir * dT * moveSpeedMultiplier;
	}
	else
	{
		move = -tCharacter->GetForwardVector() * dT * moveSpeedMultiplier;
	}

	pCharacter->SetVelocity(vel);
	*/
	//tCharacter->Translate(math::float3{ vel.x, 0.0f, vel.z });

} //end airborne movement


 // check to see if player is on the ground and its status
void Character::CheckGroundStatus()
{
	math::float3 pos{ tCharacter->GetWorldPosition() };
	pos.y = pos.y + 0.1f;
	math::float3 end{ pos.x, pos.y - 0.11f, pos.z };
	RayTestResult hitInfo = {};

	// Raytest to check if the player is standing on an object
	if (physics->RayTest(pos, end, &hitInfo))
	{
		if (hitInfo.hitWorldNormal.y < 0.9f)
		{
			moveSpeedMultiplier = slopeSpeedMultiplier;
		}
		groundNormal = hitInfo.hitWorldNormal;

		if (!isGrounded && hasJumped)
		{
			//charAudio.Stop();
			//charAudio.PlayOneShot(landFX);
			hasJumped = false;
		}
		isGrounded = true;
	}
	else
	{
		isGrounded = false;
		groundNormal = tCharacter->GetUpVector();
	}
}// end CheckGroundStatus






 //-------------------------------------------------------------------------------------------------
 // Actions


void Character::Aim() {}

void Character::Attack() {}

void Character::Dodge() {}
void Character::Die() {}

// Sprint (move faster)
void Character::Sprint(bool _sprint)
{
	// TODO
	// Set dashing to true if cool allows
	isSprinting = _sprint;
}

void Character::Special(float, bool, bool) {}





// TODO
// Check to see if forward/right need to be negated to be right
// Force the player to move a bit
void Character::ForceMove(float speed, float dT, int direction)
{
	if (direction == 0)
	{
		//tPlayer->Translate -= tPlayer->GetForwardVector() * speed * dT;
		tCharacter->Translate(tCharacter->GetForwardVector() * dT * speed);
	}
	else if (direction == 1)
	{
		//tPlayer->Translate += tPlayer->GetForwardVector() * speed * dT;
		tCharacter->Translate(tCharacter->GetForwardVector() * dT * -speed);
	}
	else if (direction == 2)
	{
		//tPlayer->Translate += tPlayer->GetRightVector() * speed * dT;
		tCharacter->Translate(tCharacter->GetRightVector() * dT * -speed);
	}
	else if (direction == 3)
	{
		//tPlayer->Translate -= tPlayer->GetRightVector() * speed * dT;
		tCharacter->Translate(tCharacter->GetRightVector() * dT * speed);
	}
}

// Force the player to move a bit
void Character::ForceMove(float speed, float dT, math::float3 direction)
{
	//tPlayer->Translate += direction * speed * dT;
	tCharacter->Translate(direction * dT * speed);

}

// Deal damage to the character
void Character::TakeDamage(float dmg)
{
	if (dmg > 0)
	{
		health -= dmg;
	}
	if (health <= 0)
	{
		isDead = true;
		Die();
	}
}

// Use special Energy
// Combo Special move (Sword/Gun)
void Character::UseSpecial(float energyUsed, bool light, bool isGun)
{
	// TODO
	// Use special energy
}

//-------------------------------------------------------------------------------------------------
// Setters

// Set the Animation Parameter
void Character::AnimationParameter(int _animationParameter)
{
	animationParameter = _animationParameter;
}

// Set Character's current state
void Character::CurrentState(CharacterState _currentState)
{
	currentState = _currentState;
}

// Set if action has an effect
void Character::HasEffect(bool _hasEffect)
{
	hasEffect = _hasEffect;
}

// Set the character's last state
void Character::LastState(CharacterState _lastState)
{
	lastState = _lastState;
}

// Set the State Timer
void Character::StateTimer(float _stateTimer)
{
	stateTimer = _stateTimer;
}

// Set transform component
void Character::SetComponents(tofu::TransformComponent _tCharacter, tofu::PhysicsComponent _pCharacter, 
	tofu::AnimationComponent _aCharacter)
{
	tCharacter = _tCharacter;
	pCharacter = _pCharacter;
	aCharacter = _aCharacter;
}

//-------------------------------------------------------------------------------------------------
// Getters

// Get the combat manager
CombatManager* Character::GetCombatManager()
{
	return combatManager;
}

// Get the character's tag
std::string Character::GetTag()
{
	return tag;
}

// Return Character Position
tofu::math::float3 Character::GetPosition()
{
	return tCharacter->GetLocalPosition();
}

// Return Character Forward
tofu::math::float3 Character::GetForward()
{
	return tCharacter->GetForwardVector();
}

// Return Character Right
tofu::math::float3 Character::GetRight()
{
	return tCharacter->GetRightVector();
}

// Return if player is grounded
bool Character::IsGrounded()
{
	return isGrounded;
}

// Return if player is dead
bool Character::IsDead()
{
	return isDead;
}

// Return if action has effect
bool Character::HasEffect()
{
	return hasEffect;
}

// Return the state timer
float Character::StateTimer()
{
	return stateTimer;
}

// Return current state
CharacterState Character::CurrentState()
{
	return currentState;
}

// Return the last state
CharacterState Character::LastState()
{
	return lastState;
}




