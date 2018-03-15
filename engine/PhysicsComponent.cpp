#include "PhysicsComponent.h"

#include <btBulletDynamicsCommon.h>

namespace tofu
{
	
	void PhysicsComponentData::SetVelocity(const math::float3& vel)
	{
		if (nullptr != rigidbody && !isStatic && !isKinematic)
		{
			rigidbody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
		}
	}

	const math::float3 PhysicsComponentData::GetVelocity()
	{
		assert(nullptr != rigidbody && !isStatic && !isKinematic);

		if (nullptr != rigidbody && !isStatic && !isKinematic)
		{
			btVector3 vel = rigidbody->getLinearVelocity();
			return tofu::math::float3{ vel.x(), vel.y(), vel.z() };
		}
		else
		{
			// Should never run.
			return tofu::math::float3{ 0.0f, 0.0f, 0.0f };
		}
	}

	void PhysicsComponentData::ApplyForce(const math::float3& force)
	{
		if (nullptr != rigidbody && !isStatic && !isKinematic)
		{
			rigidbody->applyForce(btVector3(force.x, force.y, force.z), btVector3());
		}
	}

	void PhysicsComponentData::ApplyImpulse(const math::float3 & impulse)
	{
		if (nullptr != rigidbody && !isStatic && !isKinematic)
		{
			rigidbody->applyImpulse(btVector3(impulse.x, impulse.y, impulse.z), btVector3());
		}
	}
}
