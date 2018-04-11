#include "ActionSelector.h"
#include "Character.h" // May induce circular inclusion

ActionSelector::ActionSelector() {}

ActionSelector::ActionSelector(ActionList* _actionList, CombatManager* _combatManager)
{
	actionList = _actionList;
	combatManager = _combatManager;
	combatManager->GetCurrentTarget();
}


void ActionSelector::selectNextOption(tofu::math::float3 pos, tofu::math::float3 fwd)
{

	// Check to see if I am facing the target or not.
	// Rotate
	tofu::math::float3 direc = combatManager->GetCurrentTarget()->GetPosition() - pos;
	//tofu::math::quat rot = tofu::math:: .LookRotation(direc, transform.TransformDirection(Vector3.up));
	//Quaternion.Angle(transform.rotation, new Quaternion(0, tofu::math::quat(a).y, 0, tofu::math::quat(a).w));
	float angle = tofu::math::angleBetween(fwd, direc);


	// transform.rotation = new Quaternion(0, rot.y, 0, rot.w);
	currentState.amIFacingTheTarget = (angle < 20) ? kTrue : kFalse;
	

	// Perform action based on the current State.
	currentState.hasTarget = (nullptr != combatManager->GetCurrentTarget()) ? kTrue : kFalse;

	if (!preventAttack)
	{
		currentState.canIAttack = (combatManager->GetCanAttack()) ? kTrue : kFalse;
	}

	// Can my target attack me?
	if (combatManager->GetCurrentTarget()->GetCombatManager()->GetCanAttack())
	{
		currentState.canTargetAttack = kTrue;
	}
	else
	{
		currentState.canTargetAttack = kFalse;
	}

	// Am I stunned?
	if (combatManager->GetIsHit())
	{
		currentState.amIStunned = kTrue;
	}
	else
	{
		currentState.amIStunned = kFalse;
	}
	
	// Is target stunned?
	if (combatManager->GetCurrentTarget()->GetCombatManager()->GetIsHit())
	{
		currentState.targetStunned = kTrue;
	}
	else
	{
		currentState.targetStunned = kFalse;
	}

	// TODO: Change these based on the current State.
	currentState.isTargetFacingMe = kTrue;

	// Get the valid actions for the current state.
	std::vector<Action> validActions = actionList->GetValidActions(currentState);

	// Select one of the valid actions.
	if (validActions.size > 0)
	{
		int curAction = validActions.size - 1;

		// Get the Action.
		if (combatManager->GetCanAttack())
		{
			// Check the distance and see if the Enemy can use the attack or not??

			// New Code, is the problem
			{

				//float distanceToTarget = Vector3.Distance(this.transform.position, this.combatManager.GetCurrentTarget().transform.position);
				float distanceToTarget = tofu::math::distance(pos, combatManager->GetCurrentTarget()->GetPosition());

				//float strikingDistance = this.combatManager.allMoves[Mathf.Clamp((int)((validActions[curAction]).combat) - 1, 0, 1000)].AD;
				float strikingDistance = combatManager->allMoves[tofu::math::clamp((int)(validActions[curAction].combat) - 1, 0, 1000)].AD;

				float buffer = 0.1f;

				if (tofu::math::abs(distanceToTarget - strikingDistance) > buffer)
				{
					combatManager->SetAdjustMinDistance(strikingDistance - buffer);
					combatManager->SetAdjustMaxDistance(strikingDistance + buffer);
					combatManager->SetIsAdjusting(true);
				}
				else
				{
					combatManager->PerformAction(validActions[curAction]);
					actionList->UpdateActionPreference(validActions[curAction].name, validActions[curAction].preference - 1);
				}
			}

		}
	}
	else
	{
		// This happens when you are either Dead or Stunned.
		//Debug.Log("No Valid Actions.");
	}

	// TODO: Throw an exception when no action is present..

}




//-------------------------------------------------------------------------------------------------
// Setters

void ActionSelector::SetPreventAttack(bool _preventAttack)
{
	preventAttack = _preventAttack;
}

//-------------------------------------------------------------------------------------------------
// Getters
