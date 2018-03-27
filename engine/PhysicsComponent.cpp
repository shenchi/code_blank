#include "PhysicsComponent.h"

#include "TransformComponent.h"
#include "PhysicsSystem.h"

#include <btBulletDynamicsCommon.h>

namespace tofu
{
	void PhysicsComponentData::SetGravity(const math::float3& g)
	{
		gravity = g;

		if (nullptr == rigidbody)
		{
			dirty = true;
		}
		else
		{
			rigidbody->setGravity(btVector3(g.x, g.y, g.z));
		}
	}

	const math::float3& PhysicsComponentData::GetGravity()
	{
		if (nullptr != rigidbody)
		{
			btVector3 g = rigidbody->getGravity();
			gravity = { g.x(), g.y(), g.z() };
		}

		return gravity;
	}

	void PhysicsComponentData::SetPosition(const math::float3 & pos)
	{
		if (nullptr != rigidbody)
		{
			TransformComponent t = entity.GetComponent<TransformComponent>();
			math::float3 scale = t->GetWorldScale();

			btTransform btTrans = rigidbody->getWorldTransform();

			btQuaternion btRot = btTrans.getRotation();
			math::quat rot = { btRot.w(), btRot.x(), btRot.y(), btRot.z() };

			math::float3 actualPos = pos +
				rot * (colliderDesc.origin * scale);

			btTrans.setOrigin(btVector3(actualPos.x, actualPos.y, actualPos.z));

			rigidbody->setWorldTransform(btTrans);
		}
	}

	math::float3 PhysicsComponentData::GetPosition()
	{
		if (nullptr != rigidbody)
		{
			btTransform btTrans = rigidbody->getWorldTransform();
			btVector3 btPos = btTrans.getOrigin();
			btQuaternion btRot = btTrans.getRotation();

			math::float3 scale = entity.GetComponent<TransformComponent>()->GetWorldScale();
			math::float3 pos{ float(btPos.x()), float(btPos.y()), float(btPos.z()) };
			math::quat rot(float(btRot.w()), float(btRot.x()), float(btRot.y()), float(btRot.z()));
			pos -= rot * (colliderDesc.origin * scale);
			return pos;
		}
		return math::float3();
	}

	void PhysicsComponentData::SetRotation(const math::quat & rot)
	{
		if (nullptr != rigidbody)
		{
			btTransform btTrans = rigidbody->getWorldTransform();

			btTrans.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

			rigidbody->setWorldTransform(btTrans);
		}
	}

	math::quat PhysicsComponentData::GetRotation()
	{
		if (nullptr != rigidbody)
		{
			btTransform btTrans = rigidbody->getWorldTransform();

			btQuaternion btRot = btTrans.getRotation();
			return { btRot.w(), btRot.x(), btRot.y(), btRot.z() };
		}
		return math::quat();
	}

	void PhysicsComponentData::SetVelocity(const math::float3& vel)
	{
		if (nullptr != rigidbody && !isStatic && !isKinematic)
		{
			rigidbody->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
		}
	}

	math::float3 PhysicsComponentData::GetVelocity()
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
