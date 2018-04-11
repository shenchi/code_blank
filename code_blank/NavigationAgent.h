// Darren Farr
// AI Conversion, original by Sravan Kuraturi in C#
#pragma once
#include <Tofu.h>
#include <vector>
#include "PathingNode.h"
#include "NavigationSingleton.h"
#include "Enemy.h"

// This should be aligned as close to the Unitys NavMesh Agent as possible.
class NavigationAgent
{
public:
	NavigationAgent();

	void Init(Enemy*, tofu::math::float3);
	void calculatePath();
	void SetDestination(tofu::math::float3);
	void SetIsStopped(bool);
	void Update(float, tofu::math::float3, tofu::math::float3);

	bool GetIsStopped();

private:
	NavigationSingleton* navigation;

	float AISpeedMod;

	// Timer for how long the vision would persist you lost site of the enemy.
	//[Range(0, 10)]
	float visionLingerTime = 5.0f;
	float visionLingerTimeCountDown = 0.0f;
	bool canChase = false;
	bool isChasingUsingNodes = false;

	tofu::math::float3 destination;
	int targetLayer;

	// This location is used to go back once you lose the target.
	tofu::math::float3 initialLocation;
	tofu::math::float3 position;

	std::vector<PathingNode*> path;

	// This is to check if we have to reCalculate the Path or not.
	bool reCalculatePath = false;

	// Our Current Node
	PathingNode* currentNode;
	PathingNode* targetNode;

	tofu::math::float3 rayCastSourcePoint;
	tofu::math::float3 rayCastTargetPoint;

	// Similar to the Unity NavMeshAgent.
	bool isStopped;

	float rayCastHeight;

	// Dummy Variable for debugging.
	bool canTraverseDirectly;

	//RaycastHit hitInfo;

	// Reference
	Enemy* enemyCharacter;
	float maxSensoryRadius = 0;
};