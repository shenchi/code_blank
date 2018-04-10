#include "NavigationAgent.h"


NavigationAgent::NavigationAgent() {}

void NavigationAgent::Init(Enemy* character, tofu::math::float3 initPos)
{
	initialLocation = initPos;

	enemyCharacter = character;
	assert(nullptr != enemyCharacter);
	
	maxSensoryRadius = enemyCharacter->GetMaxSensoryRadius();
	Debug.Assert(0.5 < maxSensoryRadius);
}


void Update() {

	visionLingerTimeCountDown -= Time.deltaTime;

	// This would get the Current node and set it.
	// ????????????? Node.
	currentNode = NavigationSingleton.Instance.GetCurrentNode(this.transform.position);

	if (reCalculatePath)
	{
		calculatePath();
		reCalculatePath = false;
	}

	if (canTraverseDirectly)
	{
		// Check for Rotation.
		var direc = destination - transform.position;
		var rot = Quaternion.LookRotation(direc, transform.TransformDirection(Vector3.up));
		float angle = Quaternion.Angle(transform.rotation, new Quaternion(0, rot.y, 0, rot.w));
		if (angle > 10) { transform.rotation = Quaternion.RotateTowards(transform.rotation, new Quaternion(0, rot.y, 0, rot.w), enemyCharacter.turnSpeed * Time.deltaTime); return; }

		// Rotate First.
		this.enemyCharacter.ForceMove(AISpeedMod, 1);
	}
	else
	{

		if (canChase && visionLingerTimeCountDown > 0.0f)
		{
			Debug.DrawRay(rayCastSourcePoint, (rayCastTargetPoint - rayCastSourcePoint), Color.green);
			isChasingUsingNodes = true;
			if (null != path && path.Count > 0)
			{
				this.destination = this.path[0].nodePosition;
				// Check for Rotation.
				var direc = destination - transform.position;
				var rot = Quaternion.LookRotation(direc, transform.TransformDirection(Vector3.up));
				float angle = Quaternion.Angle(transform.rotation, new Quaternion(0, rot.y, 0, rot.w));
				if (angle > 10) { transform.rotation = Quaternion.RotateTowards(transform.rotation, new Quaternion(0, rot.y, 0, rot.w), enemyCharacter.turnSpeed * Time.deltaTime); return; }

				// Rotate First.
				this.enemyCharacter.ForceMove(AISpeedMod, 1);
			}
			// Chase the Player.
		}
		else if (visionLingerTimeCountDown < 0.0f)
		{
			canChase = false;
			isChasingUsingNodes = false;
			Debug.DrawRay(rayCastSourcePoint, (rayCastTargetPoint - rayCastSourcePoint), Color.red);
		}
	}
}


void NavigationAgent::calculatePath()
{
	targetNode = NavigationSingleton.Instance.GetCurrentNode(destination);

	Debug.Assert(null != currentNode);
	Debug.Assert(null != targetNode);

	// This is an infinite loop waiting for the canCalculate Flag to be set.
	// ???? ???????????? ???? ???. ??? ??? ?????????? ???????.
	while (!NavigationSingleton.Instance.canCalculte)
	{
		continue;
	}
	path = NavigationSingleton.Instance.GetPath(currentNode, targetNode);

	Debug.Assert(null != path);
}


void SetDestination(tofu::math::float3 targetPos, int _targetLayer)
{
	//TODO: This needs to be conditional on if we want to chase using nodes or switch to targeting directly.
	this.targetLayer = _targetLayer;

	this.rayCastSourcePoint = this.transform.position;
	rayCastSourcePoint.y = rayCastHeight;
	this.rayCastTargetPoint = targetPos;
	rayCastTargetPoint.y = rayCastHeight;

	Vector3 rayDir = (rayCastTargetPoint - rayCastSourcePoint).normalized;

	// Raycast for the target, and if you can find it, we do not need the pathing nodes anymore...
	if (Physics.Raycast(rayCastSourcePoint, rayDir, out hitInfo, maxSensoryRadius))
	{
		//Debug.DrawRay(rayCastSourcePoint, (rayCastTargetPoint - rayCastSourcePoint).normalized * maxSensoryRadius, Color.blue, 0.5f);
		if (targetLayer == hitInfo.collider.transform.gameObject.layer)
		{
			this.destination = targetPos;
			Debug.DrawRay(rayCastSourcePoint, (rayCastTargetPoint - rayCastSourcePoint).normalized * maxSensoryRadius, Color.blue, 0.5f);
			canTraverseDirectly = true;
			canChase = true;
			visionLingerTimeCountDown = visionLingerTime;
			return;
		}
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