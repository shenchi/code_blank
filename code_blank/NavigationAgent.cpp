#include "NavigationAgent.h"
#include "Enemy.h"

using namespace tofu;

NavigationAgent::NavigationAgent() {}

void NavigationAgent::Init(Enemy* character, tofu::math::float3 initPos)
{
	navigation = NavigationSingleton::Instance();

	initialLocation = initPos;
	position = initialLocation;

	enemyCharacter = character;
	assert(nullptr != enemyCharacter);
	
	maxSensoryRadius = enemyCharacter->GetMaxSensoryRadius();
	assert(0.5 < maxSensoryRadius);
}


void NavigationAgent::Update(float dT, math::float3 pos, math::float3 fwd)
{
	position = pos;

	visionLingerTimeCountDown -= dT;

	// This would get the Current node and set it.

	currentNode = navigation->GetCurrentNode(position);

	if (reCalculatePath)
	{
		calculatePath();
		reCalculatePath = false;
	}

	if (canTraverseDirectly)
	{
		// Check for Rotation.
		tofu::math::float3 direc = destination - position;
		//var rot = Quaternion.LookRotation(direc, transform.TransformDirection(Vector3.up));
		//float angle = Quaternion.Angle(transform.rotation, new Quaternion(0, rot.y, 0, rot.w));
		float angle = tofu::math::angleBetween(fwd, direc);
		
		if (angle > 10) 
		{ 
			// TODO Rotate the enemy to face the player
			enemyCharacter->RotateEnemey(angle);
			
			return; 
		}

		// Rotate First.
		enemyCharacter->ForceMove(AISpeedMod, dT, 1);
	}
	else
	{
		if (canChase && visionLingerTimeCountDown > 0.0f)
		{
			isChasingUsingNodes = true;
			if (path.size() > 0)
			{
				destination = path.at(0)->GetPosition();
				// Check for Rotation.
				tofu::math::float3 direc = destination - position;
				//var rot = Quaternion.LookRotation(direc, transform.TransformDirection(Vector3.up));
				//float angle = Quaternion.Angle(transform.rotation, new Quaternion(0, rot.y, 0, rot.w));
				float angle = tofu::math::angleBetween(fwd, direc);
				
				if (angle > 10) 
				{ 
					//transform.rotation = Quaternion.RotateTowards(transform.rotation, new Quaternion(0, rot.y, 0, rot.w), enemyCharacter.turnSpeed * dT);
					enemyCharacter->RotateEnemey(angle);
					return; 
				}

				// Rotate First.
				enemyCharacter->ForceMove(AISpeedMod, dT, 1);
			}
			// Chase the Player.
		}
		else if (visionLingerTimeCountDown < 0.0f)
		{
			canChase = false;
			isChasingUsingNodes = false;
		}
	}
}


void NavigationAgent::calculatePath()
{
	targetNode = navigation->GetCurrentNode(destination);

	assert(nullptr != currentNode);
	assert(nullptr != targetNode);

	// This is an infinite loop waiting for the canCalculate Flag to be set.
	while (!navigation->GetCanCalculate())
	{
		continue;
	}
	path = navigation->GetPath(currentNode, targetNode);

	//assert(path.size() != 0);
}


void NavigationAgent::SetDestination(tofu::math::float3 targetPos)
{
	//TODO: This needs to be conditional on if we want to chase using nodes or switch to targeting directly.

	rayCastSourcePoint = position;
	rayCastSourcePoint.y = rayCastHeight;
	rayCastTargetPoint = targetPos;
	rayCastTargetPoint.y = rayCastHeight;

	tofu::math::float3 rayDir = tofu::math::normalize(rayCastTargetPoint - rayCastSourcePoint);

	// Raycast for the target, and if you can find it, we do not need the pathing nodes anymore...
	//if (Physics.Raycast(rayCastSourcePoint, rayDir, out hitInfo, maxSensoryRadius))
	if(enemyCharacter->RayCastHitPlayer(rayCastSourcePoint, rayDir, maxSensoryRadius) )
	{
		//Debug.DrawRay(rayCastSourcePoint, (rayCastTargetPoint - rayCastSourcePoint).normalized * maxSensoryRadius, Color.blue, 0.5f);

		destination = targetPos;
		canTraverseDirectly = true;
		canChase = true;
		visionLingerTimeCountDown = visionLingerTime;
		return;
	}

	// If you cannot traverse directly, and are chasing using nodes, then set the path accordingly. For now, do not modify the destination that was already set.
	if (isChasingUsingNodes)
	{
		reCalculatePath = true;
		return;
	}

	canTraverseDirectly = false;
	reCalculatePath = true;

}

//-------------------------------------------------------------------------------------------------
// Setters

void NavigationAgent::SetIsStopped(bool _isStopped)
{
	isStopped = _isStopped;
	if (true == isStopped)
	{
		canTraverseDirectly = false;
	}
}



//-------------------------------------------------------------------------------------------------
// Getters

bool NavigationAgent::GetIsStopped()
{
	return isStopped;
}