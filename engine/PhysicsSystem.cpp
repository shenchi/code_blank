#include "PhysicsSystem.h"

#include "Engine.h"
#include "RenderingSystem.h"
#include "PhysicsComponent.h"
#include "TransformComponent.h"

#include <btBulletDynamicsCommon.h>

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

	void AddToBulletTriangleMesh(btTriangleMesh* mesh, tofu::Model* model)
	{
		uint32_t numSubMesh = model->GetNumMeshes();
		uint32_t stride = model->GetStride();

		for (uint32_t iMesh = 0; iMesh < numSubMesh; iMesh++)
		{
			const uint16_t* indices = model->GetIndices(iMesh);
			const uint8_t* vertices = reinterpret_cast<const uint8_t*>(model->GetVertices(iMesh));

			uint32_t numTriangles = model->GetNumIndices(iMesh) / 3u;

			for (uint32_t iTriangle = 0; iTriangle < numTriangles; iTriangle++)
			{
				uint16_t i1 = indices[iTriangle * 3];
				uint16_t i2 = indices[iTriangle * 3 + 1];
				uint16_t i3 = indices[iTriangle * 3 + 2];

				auto& v1 = *reinterpret_cast<const tofu::math::float3*>(vertices + stride * i1);
				auto& v2 = *reinterpret_cast<const tofu::math::float3*>(vertices + stride * i2);
				auto& v3 = *reinterpret_cast<const tofu::math::float3*>(vertices + stride * i3);

				mesh->addTriangle(btVec3(v1), btVec3(v2), btVec3(v3));
			}

		}
	}

	class TofuMeshAdaptor : public btStridingMeshInterface
	{
	public:
		TofuMeshAdaptor(tofu::Model* model) : model(model) {}

		virtual void getLockedVertexIndexBase(
			unsigned char **vertexbase, 
			int& numverts, 
			PHY_ScalarType& type, 
			int& stride, 
			unsigned char **indexbase, 
			int & indexstride, 
			int& numfaces, 
			PHY_ScalarType& indicestype, 
			int subpart = 0
		) override
		{
			*vertexbase = nullptr;
			numverts = 0;
			type = PHY_FLOAT;
			stride = model->GetStride();
			*indexbase = nullptr;
			indexstride = 3;
			numfaces = 0;
			indicestype = PHY_SHORT;
		}

		virtual void getLockedReadOnlyVertexIndexBase(
			const unsigned char **vertexbase, 
			int& numverts, 
			PHY_ScalarType& type, 
			int& stride, 
			const unsigned char **indexbase, 
			int & indexstride, 
			int& numfaces, 
			PHY_ScalarType& indicestype, 
			int subpart = 0
		) const override
		{
			*vertexbase = nullptr;
			numverts = 0;
			type = PHY_FLOAT;
			stride = model->GetStride();
			*indexbase = nullptr;
			indexstride = 3;
			numfaces = 0;
			indicestype = PHY_SHORT;

			if (subpart < static_cast<int>(model->GetNumMeshes()))
			{
				*vertexbase = reinterpret_cast<const unsigned char*>(
					model->GetVertices(subpart)
					);
				*indexbase = reinterpret_cast<const unsigned char*>(
					model->GetIndices(subpart)
					);

				numverts = static_cast<int>(model->GetNumVertices(subpart));
				numfaces = static_cast<int>(model->GetNumIndices(subpart)) / 3;
			}
		}

		virtual void unLockVertexBase(int subpart) override
		{

		}

		virtual void unLockReadOnlyVertexBase(int subpart) const override
		{

		}

		virtual int	getNumSubParts() const override
		{
			return static_cast<int>(model->GetNumMeshes());
		}

		virtual void preallocateVertices(int numverts) override
		{

		}

		virtual void preallocateIndices(int numindices) override
		{

		}

	private:
		tofu::Model * model;
	};
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

		return kOK;
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
				if (nullptr != comp.meshInterface)
				{
					delete comp.meshInterface;
					comp.meshInterface = nullptr;
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

		return kOK;
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
				return kErrUnknown;
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
				if (nullptr != comp.meshInterface)
				{
					delete comp.meshInterface;
					comp.meshInterface = nullptr;
				}

				math::float3 scale = t->GetWorldScale();
				float uniformScale = 1.0f;
				if (std::abs(scale.x - scale.y) < 0.0001f && std::abs(scale.x - scale.z) < 0.0001f)
				{
					uniformScale = scale.x;
				}

				{
					math::quat rot = t->GetWorldRotation();
					math::float3 pos = t->GetWorldPosition() + 
						rot * (comp.colliderDesc.origin * scale);
					
					btTransform btTrans(btQuat(rot), btVec3(pos));
					
					switch (comp.colliderDesc.type)
					{
					case ColliderType::kColliderTypeBox:
						comp.collider = new btBoxShape(btVec3(comp.colliderDesc.halfExtends * scale));
						break;
					case ColliderType::kColliderTypeSphere:
						comp.collider = new btSphereShape(comp.colliderDesc.radius * uniformScale);
						break;
					case ColliderType::kColliderTypeCapsule:
						comp.collider = new btCapsuleShape(
							comp.colliderDesc.radius * uniformScale,
							comp.colliderDesc.height * uniformScale);
						break;
					case ColliderType::kColliderTypeCylinder:
						comp.collider = new btCylinderShape(btVec3(comp.colliderDesc.halfExtends * scale));
						break;
					case ColliderType::kColliderTypeMesh:
						if (!comp.isStatic)
							return kErrUnknown;

						//comp.meshInterface = new TofuMeshAdaptor(comp.colliderDesc.model);
						btTriangleMesh* m = new btTriangleMesh(false, false);
						AddToBulletTriangleMesh(m, comp.colliderDesc.model);
						comp.meshInterface = m;

						comp.meshInterface->setScaling(btVec3(scale));
						comp.collider = new btBvhTriangleMeshShape(comp.meshInterface, false, true);
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
								comp.lockPosX ? 0.0f : 1.0f,
								comp.lockPosY ? 0.0f : 1.0f,
								comp.lockPosZ ? 0.0f : 1.0f
							)
						);

						comp.rigidbody->setAngularFactor(
							btVector3(
								comp.lockRotX ? 0.0f : 1.0f,
								comp.lockRotY ? 0.0f : 1.0f,
								comp.lockRotZ ? 0.0f : 1.0f
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
				math::float3 scale = t->GetWorldScale();
				math::quat rot = t->GetWorldRotation();
				math::float3 pos = t->GetWorldPosition() +
					rot * (comp.colliderDesc.origin * scale);

				btTransform btTrans(btQuat(rot), btVec3(pos));
				comp.rigidbody->getMotionState()->setWorldTransform(btTrans);
			}
			else if (!comp.isStatic) // normal dynamic rigid bodies
			{
				math::float3 entityScale = t->GetWorldScale();
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
						entityRot * (comp.colliderDesc.origin * entityScale);

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
				return kErrUnknown;
			}

			comp.isCollided = false;

			if (comp.isStatic /*|| comp.isKinematic*/)
			{
				continue;
			}

			TransformComponent t = comp.entity.GetComponent<TransformComponent>();
			if (!t)
			{
				return kErrUnknown;
			}

			btTransform btTrans;
			comp.rigidbody->getMotionState()->getWorldTransform(btTrans);
			btVector3 btPos = btTrans.getOrigin();
			btQuaternion btRot = btTrans.getRotation();

			math::float3 scale = t->GetWorldScale();
			math::float3 pos{ float(btPos.x()), float(btPos.y()), float(btPos.z()) };
			math::quat rot(float(btRot.w()), float(btRot.x()), float(btRot.y()), float(btRot.z()));
			pos -= rot * (comp.colliderDesc.origin * scale);

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

		return kOK;
	}

	void PhysicsSystem::SetGravity(const math::float3 & g)
	{
		world->setGravity(btVector3(g.x, g.y, g.z));
	}

}

