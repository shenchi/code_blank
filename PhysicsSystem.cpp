#include "PhysicsSystem.h"

#include "Engine.h"
#include "PhysicsComponent.h"
#include "TransformComponent.h"

#include <btBulletDynamicsCommon.h>

namespace
{
	TF_INLINE btVector3 btVec3(const tofu::math::float3& v)
	{
		return btVector3(v.x, v.y, v.z);
	}
}

namespace tofu
{
	SINGLETON_IMPL(PhysicsSystem);

	int32_t PhysicsSystem::Init()
	{
		config = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(config);
		pairCache = new btDbvtBroadphase();
		solver = new btSequentialImpulseConstraintSolver();
		world = new btDiscreteDynamicsWorld(dispatcher,
			pairCache, solver, config);

		world->setGravity(btVector3(0, -10, 0));

		return TF_OK;
	}

	int32_t PhysicsSystem::Shutdown()
	{
		delete world;
		delete solver;
		delete pairCache;
		delete dispatcher;
		delete config;

		world = nullptr;
		solver = nullptr;
		pairCache = nullptr;
		dispatcher = nullptr;
		config = nullptr;

		return TF_OK;
	}

	int32_t PhysicsSystem::Update()
	{
		PhysicsComponentData* comps = PhysicsComponent::GetAllComponents();
		uint32_t count = PhysicsComponent::GetNumComponents();

		for (uint32_t i = 0; i < count; i++)
		{
			PhysicsComponentData& comp = comps[i];
			if (comp.dirty)
			{
				if (nullptr != comp.rigidbody)
				{
					if (comp.rigidbody->getMotionState())
					{
						delete comp.rigidbody->getMotionState();
					}
					world->removeRigidBody(comp.rigidbody);
					delete comp.rigidbody;
					comp.rigidbody = nullptr;
				}
				if (nullptr != comp.collider)
				{
					delete comp.collider;
					comp.collider = nullptr;
				}

				{
					btTransform btTrans;
					btTrans.setIdentity();

					TransformComponent t = comp.entity.GetComponent<TransformComponent>();

					math::float3 pos = t->GetWorldPosition() + comp.colliderDesc.origin;
					btTrans.setOrigin(btVec3(pos));

					switch (comp.colliderDesc.type)
					{
					case ColliderType::Box:
						comp.collider = new btBoxShape(btVec3(comp.colliderDesc.halfExtends));
						break;
					case ColliderType::Sphere:
						comp.collider = new btSphereShape(comp.colliderDesc.radius);
						break;
					case ColliderType::Capsule:
						comp.collider = new btCapsuleShape(
							comp.colliderDesc.radius,
							comp.colliderDesc.height);
						break;
					case ColliderType::Cylinder:
						comp.collider = new btCylinderShape(btVec3(comp.colliderDesc.halfExtends));
						break;
					}

					btVector3 inertia(0, 0, 0);
					float mass = 0.0f;
					if (!comp.isStatic && !comp.isKinematic)
					{
						mass = (comp.mass <= 0.0f ? 1.0f : comp.mass);
						comp.collider->calculateLocalInertia(mass, inertia);
					}

					btDefaultMotionState* motionState = new btDefaultMotionState(btTrans);
					btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, comp.collider, inertia);
					comp.rigidbody = new btRigidBody(rbInfo);
					
					if (comp.isKinematic)
					{
						comp.rigidbody->setCollisionFlags(
							comp.rigidbody->getCollisionFlags() |
							btCollisionObject::CF_KINEMATIC_OBJECT
						);

						comp.rigidbody->setActivationState(DISABLE_DEACTIVATION);
					}
				}

				comp.dirty = false;
			}
		}

		world->stepSimulation(Time::DeltaTime);


		// TODO
		return TF_OK;
	}

	void PhysicsSystem::SetGravity(const math::float3 & g)
	{
		world->setGravity(btVector3(g.x, g.y, g.z));
	}

}

