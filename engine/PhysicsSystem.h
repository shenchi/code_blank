#pragma once

#include "Module.h"
#include "TofuMath.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

namespace tofu
{
	struct RayTestResult
	{
		math::float3		hitWorldPosition;
		math::float3		hitWorldNormal;
	};

	class PhysicsSystem : public Module
	{
		SINGLETON_DECL(PhysicsSystem)

	public:
		PhysicsSystem()
			:
			config(nullptr),
			dispatcher(nullptr),
			pairCache(nullptr),
			solver(nullptr),
			world(nullptr)
		{}

	public:
		int32_t Init() override;

		int32_t Shutdown() override;

		int32_t Update() override;

		void SetGravity(const math::float3& g);

		bool RayTest(const math::float3& start, const math::float3& end, RayTestResult* result = nullptr);

	private:
		btDefaultCollisionConfiguration*		config;
		btCollisionDispatcher*					dispatcher;
		btBroadphaseInterface*					pairCache;
		btSequentialImpulseConstraintSolver*	solver;
		btDiscreteDynamicsWorld*				world;
	};
}
