#include "Engine.h"
#include "TestGame.h"

#include "RenderingSystem.h"
#include "AnimationComponent.h"
#include "InputSystem.h"

#include <btBulletDynamicsCommon.h>
#pragma comment(lib, "LinearMath_vs2010_x64_debug.lib")
#pragma comment(lib, "Bullet3Common_vs2010_x64_debug.lib")
#pragma comment(lib, "Bullet3Collision_vs2010_x64_debug.lib")
#pragma comment(lib, "Bullet3Dynamics_vs2010_x64_debug.lib")
#pragma comment(lib, "Bullet3Geometry_vs2010_x64_debug.lib")
#pragma comment(lib, "BulletCollision_vs2010_x64_debug.lib")
#pragma comment(lib, "BulletDynamics_vs2010_x64_debug.lib")


using namespace tofu;

namespace
{
	constexpr float MaxPitch = math::PI * 0.25f;
	constexpr float MinPitch = 0.0f;
	constexpr float InitPitch = math::PI * 0.125f;

	constexpr float Accelerate = 6.67f;
	constexpr float Deaccelerate = 10.0f;
	constexpr float WalkSpeed = 2.0f;
}

int32_t TestGame::Init()
{
	{
		config = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(config);
		pairCache = new btDbvtBroadphase();
		solver = new btSequentialImpulseConstraintSolver();
		world = new btDiscreteDynamicsWorld(dispatcher, 
			pairCache, solver, config);

		world->setGravity(btVector3(0, -10, 0));
	}

	{
		Entity e = Entity::Create();

		tGround = e.AddComponent<TransformComponent>();

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/ground.model");

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::OpaqueMaterial);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);

		planeShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
		btDefaultMotionState* motionState = new btDefaultMotionState();
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionState, planeShape);
		groundRb = new btRigidBody(rbInfo);

		world->addRigidBody(groundRb);
	}

	{
		Entity e = Entity::Create();

		tBox = e.AddComponent<TransformComponent>();
		tBox->SetLocalPosition(math::float3{ 0, 10, 10 });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/cube.model");

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::OpaqueMaterial);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);

		float mass = 1.0f;

		btTransform btTrans;
		btTrans.setIdentity();
		btTrans.setOrigin(btVector3(0, 10, 10));

		btVector3 inertia(0, 0, 0);

		boxShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
		boxShape->calculateLocalInertia(mass, inertia);

		btDefaultMotionState* motionState = new btDefaultMotionState(btTrans);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, boxShape, inertia);
		boxRb = new btRigidBody(rbInfo);

		world->addRigidBody(boxRb);
	}

	{
		Entity e = Entity::Create();

		tPlayer = e.AddComponent<TransformComponent>();
		tPlayer->SetLocalScale(math::float3{ 0.01f, 0.01f, 0.01f });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/archer.model");

		anim = e.AddComponent<AnimationComponent>();

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::OpaqueSkinnedMaterial);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/archer_0.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/archer_1.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);
	}

	{
		Entity e = Entity::Create();

		tCamera = e.AddComponent<TransformComponent>();

		cam = e.AddComponent<CameraComponent>();
		
		cam->SetFOV(60.0f);
		tCamera->SetLocalPosition(math::float3{ 0, 0, -2 });

		Material* skyboxMat = RenderingSystem::instance()->CreateMaterial(MaterialType::SkyboxMaterial);
		TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/craterlake.texture");
		skyboxMat->SetTexture(tex);

		cam->SetSkybox(skyboxMat);
	}

	pitch = InitPitch;
	yaw = 0.0f;

	return TF_OK;
}

int32_t TestGame::Shutdown()
{
	if (groundRb->getMotionState())
	{
		delete groundRb->getMotionState();
	}
	world->removeRigidBody(groundRb);
	delete groundRb;

	if (boxRb->getMotionState())
	{
		delete boxRb->getMotionState();
	}
	world->removeRigidBody(boxRb);
	delete boxRb;

	delete planeShape;
	delete boxShape;

	delete world;
	delete solver;
	delete pairCache;
	delete dispatcher;
	delete config;
	return TF_OK;
}

int32_t TestGame::Update()
{
	InputSystem* input = InputSystem::instance();
	if (input->IsButtonDown(ButtonId::TF_KEY_Escape))
	{
		Engine::instance()->Quit();
	}

	constexpr float sensitive = 0.01f;


	math::float3 inputDir = math::float3();

	if (input->IsGamepadConnected())
	{
		if (input->IsButtonDown(ButtonId::TF_GAMEPAD_FACE_RIGHT))
		{
			Engine::instance()->Quit();
		}

		inputDir.z = -input->GetLeftStickY();
		inputDir.x = input->GetLeftStickX();

		pitch += sensitive * input->GetRightStickY();
		yaw += sensitive * input->GetRightStickX();
	}

	pitch += sensitive * input->GetMouseDeltaY();
	yaw += sensitive * input->GetMouseDeltaX();

	if (pitch < MinPitch) pitch = MinPitch;
	if (pitch > MaxPitch) pitch = MaxPitch;


	if (input->IsButtonDown(TF_KEY_W))
	{
		inputDir.z = 1.0f;
	}
	else if (input->IsButtonDown(TF_KEY_S))
	{
		inputDir.z = -1.0f;
	}

	if (input->IsButtonDown(TF_KEY_D))
	{
		inputDir.x = 1.0f;
	}
	else if (input->IsButtonDown(TF_KEY_A))
	{
		inputDir.x = -1.0f;
	}

	math::quat camRot(pitch, yaw, 0.0f);
	math::float3 camTgt = tPlayer->GetLocalPosition() + math::float3{ 0.0f, 2.0f, 0.0f };
	math::float3 camPos = camTgt + camRot.rotate(math::float3{ 0.0f, 0.0f, -5.0f });
	
	tCamera->SetLocalPosition(camPos);
	tCamera->SetLocalRotation(camRot);

	float maxSpeed = WalkSpeed;

	if (math::length(inputDir) > 0.25f)
	{
		math::float3 moveDir = camRot.rotate(inputDir);
		moveDir.y = 0.0f;
		moveDir = math::normalize(moveDir);
		tPlayer->FaceTo(moveDir);

		speed += Time::DeltaTime * Accelerate;
		if (speed > maxSpeed)
			speed = maxSpeed;

		tPlayer->Translate(moveDir * Time::DeltaTime * speed);

		//anim->Play(1);
		anim->CrossFade(1, 0.3f);
	}
	else
	{
		speed -= Time::DeltaTime * Deaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		tPlayer->Translate(tPlayer->GetForwardVector() * Time::DeltaTime * speed);
		//anim->Play(0);
		anim->CrossFade(0, 0.2f);
	}

	{
		world->stepSimulation(Time::DeltaTime);
		btTransform btTrans;
		boxRb->getMotionState()->getWorldTransform(btTrans);
		btVector3 pos = btTrans.getOrigin();
		btQuaternion rot = btTrans.getRotation();
		
		tBox->SetLocalPosition(math::float3{ float(pos.x()), float(pos.y()), float(pos.z()) });
		tBox->SetLocalRotation(math::quat(float(rot.x()), float(rot.y()), float(rot.z()), float(rot.w())));
	}

	return TF_OK;
}
