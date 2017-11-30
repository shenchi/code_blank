#pragma once

#include "Component.h"
#include "TofuMath.h"

class btRigidBody;
class btCollisionShape;

namespace tofu
{
	class PhysicsSystem;

	enum class ColliderType
	{
		Box,
		Sphere,
		Capsule,
		Cylinder
	};

	struct ColliderDesc
	{
		ColliderType	type;
		math::float3	origin;
		union
		{
			math::float3	halfExtends;
			struct
			{
				float		radius;
				float		height;
			};
		};
	};

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
			colliderDesc(),
			isStatic(false),
			isKinematic(false),
			mass(1.0f),
			dirty(true)
		{}

		

	private:
		Entity				entity;
		btRigidBody*		rigidbody;
		btCollisionShape*	collider;
		ColliderDesc		colliderDesc;
		bool				isStatic;
		bool				isKinematic;
		float				mass;
		bool				dirty;
	};

	typedef Component<PhysicsComponentData> PhysicsComponent;
}
