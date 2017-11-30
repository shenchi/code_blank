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
		union
		{
			struct
			{
				math::float3	origin;
				math::float3	halfExtends;
			} Box;

			struct
			{
				math::float3	origin;
				float			radius;
			} Sphere;

			struct
			{
				math::float3	origin;
				float			radius;
				float			height;
			} Capsule;

			struct
			{
				math::float3	origin;
				math::float3	halfExtends;
			} Cylinder;
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
			mass(1.0f),
			dirty(true)
		{}

		

	private:
		Entity				entity;
		btRigidBody*		rigidbody;
		btCollisionShape*	collider;
		ColliderDesc		colliderDesc;
		float				mass;
		bool				dirty;
	};

	typedef Component<PhysicsComponentData> PhysicsComponent;
}
