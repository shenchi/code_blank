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
void GameplayAnimationMachine::Play(CharacterState state, float duration, size_t layerMask)
{
	//Debug.Log(state);
	switch (state)
	{
	case kNoState:
		return;
	case kIdleOutCombat:
		aComp->CrossFade("idle", 0.05f, 0);
		aComp->CrossFade("idle", 0.05f, 1);
		break;
	case kIdleInCombat:
		aComp->CrossFade("combat_idle", duration, 0);
		aComp->CrossFade("combat_idle", duration, 1);
		break;
	case kInjuredIdle:
		aComp->CrossFade("kInjuredIdle", 0.05f, 0);
		aComp->CrossFade("kInjuredIdle", 0.05f, 1);
		break;
	case kWalk:
		aComp->CrossFade("walk", 0.2f, 0);
		aComp->CrossFade("walk", 0.2f, 1);
		break;
	case kInjuredWalk:
		aComp->CrossFade("kInjuredWalk", 0.2f, 0);
		aComp->CrossFade("kInjuredWalk", 0.2f, 1);
		break;
	case kRun:
		aComp->CrossFade("run", 0.1f, 0);
		aComp->CrossFade("run", 0.1f, 1);
		break;
	case kJumpingPrepare:
		//aComp->CrossFade("kRunJump", 0.05f, 0);
		//aComp->CrossFade("kRunJump", 0.05f, 1);
		aComp->CrossFade("jump_up", 0.05f, 0);
		aComp->CrossFade("jump_up", 0.05f, 1);
		break;
	case kJumpUp:
		aComp->CrossFade("jump", 0.5f, 0);
		aComp->CrossFade("jump", 0.5f, 1);
		break;
	case kJumpAir:
		aComp->CrossFade("jump_air", 0.1f, 0);
		aComp->CrossFade("jump_air", 0.1f, 1);
		break;
	case kJumpDown:
		aComp->CrossFade("jump_down", 0.1f, 0);
		aComp->CrossFade("jump_down", 0.1f, 1);
		break;
	case kDead:
		aComp->CrossFade("kHitHeadWeak", 0.5f, 0);
		aComp->CrossFade("kHitHeadWeak", 0.5f, 1);
		//aComp->CrossFade("kDeath", 0.5f, 0);
		//aComp->CrossFade("kDeath", 0.5f, 1);
		break;
	case kRoll:
		aComp->CrossFade("kRoll", 0.5f, 0);
		aComp->CrossFade("kRoll", 0.5f, 1);
		break;
	case kAttack:
		{
			switch (combatManager->GetCurrentCombat())
			{
			case kNone:
				aComp->CrossFade("", -1, 0);
				aComp->CrossFade("", -1, 1);
				break;
			case kPunchJabL:
				aComp->CrossFade("kPunchJabL", 0.2f, 0);
				aComp->CrossFade("kPunchJabL", 0.2f, 1);
				break;
			case kPunchJabR:
				aComp->CrossFade("kPunchJabR", 0.2f, 0);
				aComp->CrossFade("kPunchJabR", 0.2f, 1);
				break;
			case kPunchHookL:
				aComp->CrossFade("kPunchHookL", 0.2f, 0);
				aComp->CrossFade("kPunchHookL", 0.2f, 1);
				break;
			case kPunchHookR:
				aComp->CrossFade("kPunchHookR", 0.2f, 0);
				aComp->CrossFade("kPunchHookR", 0.2f, 1);
				break;
			case kPunchUpperCutL:
				assert(false);
				aComp->CrossFade("kPunchUpperCutL", 0.2f, 0);
				aComp->CrossFade("kPunchUpperCutL", 0.2f, 1);
				break;
			case kPunchUpperCutR:
				assert(false);
				aComp->CrossFade("kPunchUpperCutR", 0.2f, 0);
				aComp->CrossFade("kPunchUpperCutR", 0.2f, 1);
				break;
			case kKickStraightMidR:
				// 5 Basic Attacks
				aComp->CrossFade("kKickStraightMidR", 0.2f, 0);
				aComp->CrossFade("kKickStraightMidR", 0.2f, 1);
				break;
			case kKickKnee:
				// Currently not used
				//aComp->CrossFade("kKickKnee", 1, 0);
				assert(false);
				break;
			case kKickAxeKick:
				// Press and Hold Attack
				aComp->CrossFade("kKickAxeKick", 0.2f, 0);
				aComp->CrossFade("kKickAxeKick", 0.2f, 1);
				break;
			case kKickHorseKick:
				// 5 Basic attacks and then hold attack
				aComp->CrossFade("kKickHorseKick", 0.2f, 0);
				aComp->CrossFade("kKickHorseKick", 0.2f, 1);
				break;
			case kSwordAttackR:
				aComp->CrossFade("kSwordR", 0.3f, 0);
				aComp->CrossFade("kSwordR", 0.3f, 1);
				break;
			case kSwordAttackRL:
				aComp->CrossFade("kSwordR2", 0.08f, 0);
				aComp->CrossFade("kSwordR2", 0.08f, 1);
				break;
			case kSwordAttackSpU:
				aComp->CrossFade("kSwordCombo", 0.5f, 0);
				aComp->CrossFade("kSwordCombo", 0.5f, 1);
				break;
			case kSwordAttackComboLL:
				aComp->CrossFade("kSwordCombo", 0.2f, 0);
				aComp->CrossFade("kSwordCombo", 0.2f, 1);
				break;
			case kNumberOfItems:
				aComp->CrossFade("", 0.2f, 0);
				aComp->CrossFade("", 0.2f, 1);
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