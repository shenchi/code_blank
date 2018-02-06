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
	case idle_OutCombat:
		aComp->CrossFade("Idle_OutCombat", 0.2f, layerMask);
		break;
	case idle_InCombat:
		aComp->CrossFade("Idle_InCombat", 0.2f, layerMask);
		break;
	case run:
		if (parameter == 0)
		{
			aComp->CrossFade("Run", 0.1f, layerMask);
		}
		else {
			aComp->Play("Sprint", layerMask);
		}
		break;
	case jump_up:
		aComp->Play("Jump_Up", layerMask);
		break;
	case jump_air:
		aComp->Play("Jump_Air", layerMask);
		break;
	case jump_down:
		aComp->Play("Jump_Down", layerMask);
		break;
	case draw_Gun:
		aComp->Play("Gun_Draw", layerMask);
		break;
	case holster_Gun:
		aComp->Play("Gun_Holster", layerMask);
		break;
	case shoot:
		aComp->Play("Gun_Shoot", layerMask);
		break;
	case aim_Idle:
		aComp->Play("Gun_Idle", layerMask);
		break;
	case aim_Move:
		if (parameter == 0)
		{
			aComp->Play("Aim_Sidestep_L", layerMask);
		}
		else if (parameter == 1)
		{
			aComp->Play("Aim_Sidestep_R", layerMask);
		}
		else if (parameter == 2)
		{
			aComp->Play("Aim_WalkB", layerMask);
		}
		else if (parameter == 3)
		{
			aComp->Play("Aim_WalkF", layerMask);
		}
		break;
	case dodge:
		if (parameter == 0)
		{
			aComp->Play("Dodge_L", layerMask);
		}
		else if (parameter == 1)
		{
			aComp->Play("Dodge_R", layerMask);
		}
		else if (parameter == 2)
		{
			aComp->Play("Dodge_B", layerMask);
		}
		else if (parameter == 3)
		{
			aComp->Play("Dodge_F", layerMask);
		}
		break;
	case dead:
		aComp->Play("Death", layerMask);
		break;
	case roll:
		aComp->Play("Roll", layerMask);
		break;
	case attack:
		combatManager->combat = (combatManager.Combat)parameter;
		aComp->Play(combat.ToString(), -1, 0);
		break;
	case adjustPosition:
		aComp->Play("Adjust");
		break;
	case hit:
		combatManager.HitPosition pos = (combatManager.HitPosition)((parameter / 100) % 10);
		combatManager.HitDirection dir = (combatManager.HitDirection)((parameter / 10) % 10);
		combatManager.HitPower power = (combatManager.HitPower)((parameter / 1) % 10);
		string animationName = "A_Hit_" + pos.ToString() + "_" + dir.ToString() + "_" + power.ToString();
		aComp->Play(animationName, -1, 0);
		break;
	}
}