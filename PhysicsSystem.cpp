#include "PhysicsSystem.h"

#include "Engine.h"
#include "PhysicsComponent.h"
#include "TransformComponent.h"

#include <btBulletDynamicsCommon.h>

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
					btTrans.setOrigin(btVector3(0, 10, 10));

					btVector3 inertia(0, 0, 0);

					boxShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
					boxShape->calculateLocalInertia(mass, inertia);

					btDefaultMotionState* motionState = new btDefaultMotionState(btTrans);
					btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, boxShape, inertia);
					boxRb = new btRigidBody(rbInfo);
				}

				comp.dirty = false;
			}
		}

		world->stepSimulation(Time::DeltaTime);


		// TODO
		return TF_OK;
	}

}

