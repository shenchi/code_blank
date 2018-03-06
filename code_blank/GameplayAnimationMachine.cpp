#include "GameplayAnimationMachine.h"
#include "CombatManager.h"

// Constructor
GameplayAnimationMachine::GameplayAnimationMachine( CombatManager* _combatManager)
{
	combatManager = _combatManager;
}

GameplayAnimationMachine::~GameplayAnimationMachine()
{}

void GameplayAnimationMachine::SetAnimComp(tofu::AnimationComponent _aComp)
{
	aComp = _aComp;
}

// 
void GameplayAnimationMachine::Play(CharacterState state, uint32_t parameter, size_t layerMask)
{
	//Debug.Log(state);
	switch (state)
	{
	case kIdleOutCombat:
		aComp->CrossFade("idle", 0.05f, 0);
		//aComp->CrossFade("Idle_OutCombat", 0.2f, layerMask);
		//aComp->CrossFade(0, 0.2f);
		break;
	case kIdleInCombat:
		aComp->CrossFade("combat_idle", 0.03f, 0);
		//aComp->Play("idle", 0);
		//aComp->CrossFade("Idle_InCombat", 0.2f, layerMask);
		break;
	case kWalk:
		aComp->CrossFade("walk", 0.2f, 0);
		//aComp->CrossFade("Run", 0.1f, layerMask);
		//aComp->CrossFade(1, 0.2f);
		break;
	case kRun:
		aComp->CrossFade("run", 0.1f, 0);
		//aComp->Play("Sprint", layerMask);
		//aComp->CrossFade(2, 0.3f);
		break;
	case kJumpUp:
		aComp->CrossFade("jump", 0.5f, 0);
		//aComp->CrossFade("jump_up", 0.2f, 0);
		//aComp->Play("Jump_Up", layerMask);
		//aComp->CrossFade(3, 0.5f);
		break;
	case kJumpAir:
		//aComp->CrossFade("jump_air", 0.1f, 0);
		//aComp->Play("Jump_Air", layerMask);
		break;
	case kJumpDown:
		//aComp->CrossFade("jump_down", 0.1f, 0);
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
		//combatManager->combat = (combatManager->Combat)parameter;
		{
			switch (combatManager->GetCurrentCombat())
			{
			case kNone:
				aComp->CrossFade("", -1, 0);
				break;
			case kPunchJabL:
				aComp->CrossFade("kPunchJabL", 0.2f, 0);
				break;
			case kPunchJabR:
				aComp->CrossFade("kPunchJabR", 0.2f, 0);
				break;
			case kPunchHookL:
				aComp->CrossFade("kPunchHookL", 0.2f, 0);
				break;
			case kPunchHookR:
				aComp->CrossFade("kPunchHookR", 0.2f, 0);
				break;
			case kPunchUpperCutL:
				assert(false);
				aComp->CrossFade("kPunchUpperCutL", 0.2f, 0);
				break;
			case kPunchUpperCutR:
				assert(false);
				aComp->CrossFade("kPunchUpperCutR", 0.2f, 0);
				break;
			case kKickStraightMidR:
				aComp->CrossFade("kKickStraightMidR", 0.2f, 0);
				break;
			case kKickKnee:
				aComp->CrossFade("kKickKnee", 1, 0);
				break;
			case kKickAxeKick:
				aComp->CrossFade("kKickAxeKick", 0.2f, 0);
				break;
			case kKickHorseKick:
				aComp->CrossFade("kKickHorseKick", 0.2f, 0);
				break;
			case kSwordAttackR:
				aComp->CrossFade("", 0.2f, 0);
				break;
			case kSwordAttackRL:
				aComp->CrossFade("", 0.2f, 0);
				break;
			case kSwordAttackSpU:
				aComp->CrossFade("", 0.2f, 0);
				break;
			case kSwordAttackComboLL:
				aComp->CrossFade("", 0.2f, 0);
				break;
			case kGunShoot:
				aComp->CrossFade("", 0.2f, 0);
				break;
			case kNumberOfItems:
				aComp->CrossFade("", 0.2f, 0);
				break;
			default:
				break;
			}
		}
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