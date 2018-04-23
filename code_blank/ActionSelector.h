// Darren Farr
// AI Conversion, original by Sravan Kuraturi in C#
#pragma once
#include "ActionList.h"
#include "CombatManager.h"

class ActionSelector
{
public:

	ActionSelector();
	ActionSelector(ActionList*, CombatManager*);

	void selectNextOption(tofu::math::float3, tofu::math::float3);
	void SetPreventAttack(bool);




private:

	ActionList* actionList;
	CombatManager* combatManager;

	bool preventAttack = false;

	State currentState;

};