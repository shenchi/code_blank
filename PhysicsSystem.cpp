#include "PhysicsSystem.h"

#include "Engine.h"
#include "PhysicsComponent.h"
#include "TransformComponent.h"

#include <btBulletDynamicsCommon.h>
#pragma comment(lib, "LinearMath_vs2010_x64_debug.lib")
#pragma comment(lib, "Bullet3Common_vs2010_x64_debug.lib")
#pragma comment(lib, "Bullet3Collision_vs2010_x64_debug.lib")
#pragma comment(lib, "Bullet3Dynamics_vs2010_x64_debug.lib")
#pragma comment(lib, "Bullet3Geometry_vs2010_x64_debug.lib")
#pragma comment(lib, "BulletCollision_vs2010_x64_debug.lib")
#pragma comment(lib, "BulletDynamics_vs2010_x64_debug.lib")

namespace
{
	TF_INLINE btVector3 btVec3(const tofu::math::float3& v)
	{
		return btVector3(v.x, v.y, v.z);
	}

	TF_INLINE btQuaternion btQuat(const tofu::math::quat& q)
	{
		return btQuaternion(q.x, q.y, q.z, q.w);
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
		PhysicsComponentData* comps = PhysicsComponent::GetAllComponents();
		uint32_t count = PhysicsComponent::GetNumComponents();

		for (uint32_t i = 0; i < count; i++)
		{
			PhysicsComponentData& comp = comps[i];
			if (nullptr != comp.rigidbody)
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
			}
		}

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

			TransformComponent t = comp.entity.GetComponent<TransformComponent>();
			if (!t)
			{
				return TF_UNKNOWN_ERR;
			}

			// create or re-create collision shape and rigidbody if necessary
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
					math::quat rot = t->GetWorldRotation();
					math::float3 pos = t->GetWorldPosition() + 
						rot.rotate(comp.colliderDesc.origin);
					
					btTransform btTrans(btQuat(rot), btVec3(pos));
					
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
					else
					{
						comp.rigidbody->setLinearFactor(
							btVector3(
								comp.lockPosX ? 0 : 1,
								comp.lockPosY ? 0 : 1,
								comp.lockPosZ ? 0 : 1
							)
						);

						comp.rigidbody->setAngularFactor(
							btVector3(
								comp.lockRotX ? 0 : 1,
								comp.lockRotY ? 0 : 1,
								comp.lockRotZ ? 0 : 1
							)
						);
					}

					comp.rigidbody->setUserIndex(comp.entity.id);
					world->addRigidBody(comp.rigidbody);
				}

				comp.dirty = false;
			}
			else if (comp.isKinematic)
			{
				math::quat rot = t->GetWorldRotation();
				math::float3 pos = t->GetWorldPosition() +
					rot.rotate(comp.colliderDesc.origin);

				btTransform btTrans(btQuat(rot), btVec3(pos));
				comp.rigidbody->getMotionState()->setWorldTransform(btTrans);
			}
			else if (!comp.isStatic) // normal dynamic rigid bodies
			{
				math::quat entityRot = t->GetWorldRotation();
				math::float3 entityPos = t->GetWorldPosition();

				// if it moved since last frame
				if (entityRot.x != comp.lastWorldRotation.x ||
					entityRot.y != comp.lastWorldRotation.y ||
					entityRot.z != comp.lastWorldRotation.z ||
					entityRot.w != comp.lastWorldRotation.w ||
					entityPos.x != comp.lastWorldPosition.x ||
					entityPos.y != comp.lastWorldPosition.y ||
					entityPos.z != comp.lastWorldPosition.z)
				{
					comp.rigidbody->activate();

					math::float3 pos = entityPos +
						entityRot.rotate(comp.colliderDesc.origin);

					btTransform btTrans(btQuat(entityRot), btVec3(pos));
					comp.rigidbody->setWorldTransform(btTrans);
				}
			}
		}

		world->stepSimulation(Time::DeltaTime, 10);

		for (uint32_t i = 0; i < count; i++)
		{
			PhysicsComponentData& comp = comps[i];
			if (nullptr == comp.rigidbody)
			{
				return TF_UNKNOWN_ERR;
			}

			comp.isCollided = false;

			if (comp.isStatic /*|| comp.isKinematic*/)
			{
				continue;
			}

			TransformComponent t = comp.entity.GetComponent<TransformComponent>();
			if (!t)
			{
				return TF_UNKNOWN_ERR;
			}

			btTransform btTrans;
			comp.rigidbody->getMotionState()->getWorldTransform(btTrans);
			btVector3 btPos = btTrans.getOrigin();
			btQuaternion btRot = btTrans.getRotation();

			math::float3 pos{ float(btPos.x()), float(btPos.y()), float(btPos.z()) };
			math::quat rot(float(btRot.x()), float(btRot.y()), float(btRot.z()), float(btRot.w()));
			pos -= rot.rotate(comp.colliderDesc.origin);

			// TODO world position & rotation actually
			t->SetLocalPosition(pos);
			t->SetLocalRotation(rot);

			comp.lastWorldPosition = pos;
			comp.lastWorldRotation = rot;
			
		}

		int numManifolds = dispatcher->getNumManifolds();
		for (int i = 0; i < numManifolds; i++)
		{
			btPersistentManifold* manifold = dispatcher->getManifoldByIndexInternal(i);

			if (manifold->getNumContacts() > 0)
			{
				Entity e0(manifold->getBody0()->getUserIndex());
				Entity e1(manifold->getBody1()->getUserIndex());
				e0.GetComponent<PhysicsComponent>()->isCollided = true;
				e1.GetComponent<PhysicsComponent>()->isCollided = true;
			}
		}

		return TF_OK;
	}

	void PhysicsSystem::SetGravity(const math::float3 & g)
	{
		world->setGravity(btVector3(g.x, g.y, g.z));
	}

}

