#include "GameplayAnimationMachine.h"

// Constructor
GameplayAnimationMachine::GameplayAnimationMachine(tofu::AnimationComponent _aComp, CombatManager* _combatManager)
{
	aComp = _aComp;
	combatManager = _combatManager;
}

// 
void GameplayAnimationMachine::Play(CharacterState state, uint32_t parameter, size_t layerMask)
{
	//Debug.Log(state);
	switch (state)
	{
	case kIdleOutCombat:
		//aComp->CrossFade("Idle_OutCombat", 0.2f, layerMask);
		aComp->CrossFade(0, 0.2f);
		break;
	case kIdleInCombat:
		//aComp->CrossFade("Idle_InCombat", 0.2f, layerMask);
		aComp->CrossFade(0, 0.2f); // Temp, is wrong animation
		break;
	case kRun:
		if (parameter == 0)
		{
			//aComp->CrossFade("Run", 0.1f, layerMask);
			aComp->CrossFade(1, 0.2f);
		}
		else {
			//aComp->Play("Sprint", layerMask);
			aComp->CrossFade(2, 0.3f);
		}
		break;
	case kJumpUp:
		//aComp->Play("Jump_Up", layerMask);
		aComp->CrossFade(3, 0.5f);
		break;
	case kJumpAir:
		//aComp->Play("Jump_Air", layerMask);
		break;
	case kJumpDown:
		//aComp->Play("Jump_Down", layerMask);
		break;
	case kDrawGun:
		//aComp->Play("Gun_Draw", layerMask);
		break;
	case kHolsterGun:
		//aComp->Play("Gun_Holster", layerMask);
		break;
	case kShoot:
		//aComp->Play("Gun_Shoot", layerMask);
		break;
	case kAimIdle:
		//aComp->Play("Gun_Idle", layerMask);
		break;
	case kAimMove:
		if (parameter == 0)
		{
			//aComp->Play("Aim_Sidestep_L", layerMask);
		}
		else if (parameter == 1)
		{
			//aComp->Play("Aim_Sidestep_R", layerMask);
		}
		else if (parameter == 2)
		{
			//aComp->Play("Aim_WalkB", layerMask);
		}
		else if (parameter == 3)
		{
			//aComp->Play("Aim_WalkF", layerMask);
		}
		break;
	case kDodge:
		if (parameter == 0)
		{
			//aComp->Play("Dodge_L", layerMask);
		}
		else if (parameter == 1)
		{
			//aComp->Play("Dodge_R", layerMask);
		}
		else if (parameter == 2)
		{
			//aComp->Play("Dodge_B", layerMask);
		}
		else if (parameter == 3)
		{
			//aComp->Play("Dodge_F", layerMask);
		}
		break;
	case kDead:
		//aComp->Play("Death", layerMask);
		break;
	case kRoll:
		//aComp->Play("Roll", layerMask);
		break;
	case kAttack:
		//combatManager->combat = (combatManager.Combat)parameter;
		//aComp->Play(combat.ToString(), -1, 0);
		break;
	case kAdjustPosition:
		//aComp->Play("Adjust");
		break;
	case kHit:
		/*combatManager.HitPosition pos = (combatManager.HitPosition)((parameter / 100) % 10);
		combatManager.HitDirection dir = (combatManager.HitDirection)((parameter / 10) % 10);
		combatManager.HitPower power = (combatManager.HitPower)((parameter / 1) % 10);
		string animationName = "A_Hit_" + pos.ToString() + "_" + dir.ToString() + "_" + power.ToString();
		aComp->Play(animationName, -1, 0);*/
		break;
	}
}