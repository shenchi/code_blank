#pragma once

#include "Module.h"

#include "Entity.h"
#include "TransformComponent.h"
#include "CameraComponent.h"
#include "RenderingComponent.h"
#include "AnimationComponent.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;

class btStaticPlaneShape;
class btBoxShape;

class TestGame : public tofu::Module
{
public:
	virtual int32_t Init() override;

	virtual int32_t Shutdown() override;

	virtual int32_t Update() override;

private:
	tofu::TransformComponent	tGround;
	tofu::TransformComponent	tBox;
	tofu::TransformComponent	tPlayer;
	tofu::TransformComponent	tCamera;
	tofu::AnimationComponent	anim;
	tofu::CameraComponent		cam;
	float pitch;
	float yaw;
	float speed;

private:
	btDefaultCollisionConfiguration*		config;
	btCollisionDispatcher*					dispatcher;
	btBroadphaseInterface*					pairCache;
	btSequentialImpulseConstraintSolver*	solver;
	btDiscreteDynamicsWorld*				world;

	btRigidBody*				groundRb;
	btRigidBody*				boxRb;
	btStaticPlaneShape*			planeShape;
	btBoxShape*					boxShape;
	
};
