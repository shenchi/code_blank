#pragma once

#include "Component.h"
#include "TofuMath.h"

class btRigidBody;
class btCollisionShape;
class btStridingMeshInterface;

namespace tofu
{
	class PhysicsSystem;
	class Model;

	enum class ColliderType
	{
		kColliderTypeBox,
		kColliderTypeSphere,
		kColliderTypeCapsule,
		kColliderTypeCylinder,
		kColliderTypeMesh,
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
			Model*			model;
		};

		ColliderDesc()
			:
			type(ColliderType::kColliderTypeBox),
			origin(),
			halfExtends{ 0.5f, 0.5f, 0.5f }
		{}
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
			meshInterface(nullptr),
			colliderDesc(),
			isStatic(false),
			isKinematic(false),
			mass(1.0f),
			lockPosX(false),
			lockPosY(false),
			lockPosZ(false),
			lockRotX(false),
			lockRotY(false),
			lockRotZ(false),
			lastWorldPosition(),
			lastWorldRotation(),
			isCollided(false),
			dirty(true)
		{}

	public:

		void SetMass(float m) { mass = m; dirty = true; }

		float GetMass() const { return mass; }

		void SetStatic(bool s) { isStatic = s; dirty = true; }

		bool IsStatic() const { return isStatic; }

		void SetKinematic(bool k) { isKinematic = k; dirty = true; }

		bool IsKinematic() const { return isKinematic; }

		void SetColliderOrigin(const math::float3& origin) { colliderDesc.origin = origin; dirty = true; }

		const math::float3& GetColliderOrigin() const { return colliderDesc.origin; }

		void SetBoxCollider(const math::float3& halfExtends)
		{
			colliderDesc.type = ColliderType::kColliderTypeBox;
			colliderDesc.halfExtends = halfExtends;
			dirty = true;
		}

		void SetSphereCollider(float radius)
		{
			colliderDesc.type = ColliderType::kColliderTypeSphere;
			colliderDesc.radius = radius;
			dirty = true;
		}

		void SetCapsuleCollider(float radius, float height)
		{
			colliderDesc.type = ColliderType::kColliderTypeCapsule;
			colliderDesc.radius = radius;
			colliderDesc.height = height;
			dirty = true;
		}

		void SetCylinderCollider(const math::float3& halfExtends)
		{
			colliderDesc.type = ColliderType::kColliderTypeCylinder;
			colliderDesc.halfExtends = halfExtends;
			dirty = true;
		}

		void SetMeshCollider(Model* model)
		{
			colliderDesc.type = ColliderType::kColliderTypeMesh;
			colliderDesc.model = model;
			dirty = true;
		}

		ColliderDesc GetColliderDesc() const { return colliderDesc; }

		void LockPosition(bool x, bool y, bool z)
		{
			lockPosX = x;
			lockPosY = y;
			lockPosZ = z;
			dirty = true;
		}

		void LockRotation(bool x, bool y, bool z)
		{
			lockRotX = x;
			lockRotY = y;
			lockRotZ = z;
			dirty = true;
		}

		bool IsCollided() const { return isCollided; }

		void SetVelocity(const math::float3& vel);
		const math::float3 GetVelocity();
		void ApplyForce(const math::float3& force);
		void ApplyImpulse(const math::float3& impulse);

	private:
		Entity				entity;
		btRigidBody*		rigidbody;
		btCollisionShape*	collider;
		btStridingMeshInterface*	meshInterface;
		ColliderDesc		colliderDesc;
		bool				isStatic;
		bool				isKinematic;
		float				mass;

		bool				lockPosX;
		bool				lockPosY;
		bool				lockPosZ;

		bool				lockRotX;
		bool				lockRotY;
		bool				lockRotZ;

		math::float3		lastWorldPosition;
		math::quat			lastWorldRotation;

		bool				isCollided;

		bool				dirty;
	};

	typedef Component<PhysicsComponentData> PhysicsComponent;
}
