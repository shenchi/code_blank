#pragma once

#include "Component.h"

class btRigidBody;
class btCollisionShape;

namespace tofu
{
	class PhysicsSystem;

	class PhysicsComponentData
	{
		friend class PhysicsSystem;

	public:
		PhysicsComponentData() : PhysicsComponentData(Entity()) {}

		PhysicsComponentData(Entity e)
			:
			entity(e),
			rigidbody(nullptr),
			collider(nullptr),
			dirty(true)
		{}

	private:
		Entity				entity;
		btRigidBody*		rigidbody;
		btCollisionShape*	collider;
		bool				dirty;
	};

	typedef Component<PhysicsComponentData> PhysicsComponent;
}
