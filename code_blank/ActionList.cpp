#include "ActionList.h"
#include <algorithm>

ActionList::ActionList() {}



void ActionList::UpdateActionPreference(std::string actionName, int preference)
{
	for (int i = 0; i < initalActionsList.size; i++)
	{
		if (actionName == initalActionsList[i].name)
		{
			initalActionsList[i].preference = preference;
			// I do not understand why a new action needs to be created to update the actions preference?
			//Action newAction;
			//newAction = initalActionsList[i];
			//newAction.preference = preference;
			//initalActionsList[i] = newAction;
		}
	}
}


bool ActionList::Compatible(State needState, State givenState)
{
	// Check all the parameters and see if it is Compatible.

	if (!Compatible(needState.hasTarget, givenState.hasTarget)) return false;
	if (!Compatible(needState.canTargetAttack, givenState.canTargetAttack)) return false;
	if (!Compatible(needState.canIAttack, givenState.canIAttack)) return false;
	if (!Compatible(needState.targetStunned, givenState.targetStunned)) return false;
	if (!Compatible(needState.amIStunned, givenState.amIStunned)) return false;
	if (!Compatible(needState.isTargetFacingMe, givenState.isTargetFacingMe)) return false;
	if (!Compatible(needState.amIFacingTheTarget, givenState.amIFacingTheTarget)) return false;

	return true;
}


bool ActionList::Compatible(customBool needed, customBool given)
{
	if (needed == kDoesntMatter)
	{
		return true;
	}
	else if (needed == given)
	{
		return true;
	}
	return false;
}

// Get the valid actions for the given state.
std::vector<Action> ActionList::GetValidActions(State currentState)
{

	std::vector<Action> validActions;

	for (int i = 0; i < initalActionsList.size; i++)
	{

		if (Compatible(initalActionsList[i].intialState, currentState))
		{
			validActions.push_back(initalActionsList[i]);
		}
	}

	// Sort all the actions by the preference..
	// TODO Does this run on PS4???
	std::sort(validActions.begin(), validActions.end(), CompareMyType);//((a, b) = > (a.preference - b.preference));

	// Return the Valid ACtions. This could be an empty string.
	return validActions;
}


int ActionList::CompareMyType(Action a, Action b)
{
	if (a.preference < b.preference) return -1;
	if (a.preference == b.preference) return 0;
	if (a.preference > b.preference) return 1;
}
