// Darren Farr
// AI Conversion, original by Sravan Kuraturi in C#
#pragma once
#include <vector>
#include "GameplayDS.h"




class ActionList
{
public:

	ActionList();

	void UpdateActionPreference(std::string, int);

	bool Compatible(State needState, State givenState);
	bool Compatible(customBool needed, customBool given);
	int CompareMyType(Action, Action);
	std::vector<Action> GetValidActions(State);

private:

	std::vector<Action> initalActionsList;
};