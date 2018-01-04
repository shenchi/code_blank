#include "Enemy.h"
#include <RenderingComponent.h>

using namespace tofu;

Enemy::Enemy(tofu::math::float3 pos)
{
	{
		Entity e = Entity::Create();

		tEnemy = e.AddComponent<TransformComponent>();
		tEnemy->SetLocalPosition(pos);
		tEnemy->SetLocalScale(math::float3{ 0.01f, 0.01f, 0.01f });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/archer.model");

		aEnemy = e.AddComponent<AnimationComponent>();

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaqueSkinned);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/archer_0.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/archer_1.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);

		pEnemy = e.AddComponent<PhysicsComponent>();

		pEnemy->LockRotation(true, false, true);
		pEnemy->SetCapsuleCollider(0.5f, 1.0f);
		pEnemy->SetColliderOrigin(math::float3{ 0.0f, 1.0f, 0.0f });
	}

	walkSpeed = 2.0f;
	dashSpeed = 4.0f;
}

Enemy::~Enemy() {}

void Enemy::Update()
{
	inAir = !pEnemy->IsCollided();

	// TODO
	// Handle the reset of dashing here
}

// Enemy Movement
void Enemy::MoveReg(float dT, bool jump, math::float3 inputDir, math::quat camRot)
{
	Update();

	if (math::length(inputDir) > 0.25f)
	{
		math::float3 moveDir = camRot * inputDir;
		moveDir.y = 0.0f;
		moveDir = math::normalize(moveDir);
		tEnemy->FaceTo(moveDir);

		speed += dT * kAccelerate;

		if (!isDashing)
		{
			if (speed > walkSpeed)
				speed = walkSpeed;
		}
		else
		{
			if (speed > dashSpeed)
				speed = dashSpeed;
		}

		if (speed > kMaxSpeed)
			speed = kMaxSpeed;

		tEnemy->Translate(moveDir * dT * speed);

		aEnemy->CrossFade(1, 0.3f);
	}
	else
	{
		speed -= dT * kDeaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		tEnemy->Translate(tEnemy->GetForwardVector() * dT * speed);

		aEnemy->CrossFade(0, 0.2f);
	}

	if (jump && !inAir)
	{
		pEnemy->ApplyImpulse(math::float3{ 0.0f, 2.0f, 0.0f });
	}
}

// Enemy Movement in Aiming Mode
void Enemy::MoveAim(float dT, tofu::math::float3 inputDir, math::quat camRot, tofu::math::float3 camFwd)
{
	Update();

	if (math::length(inputDir) > 0.25f)
	{
		math::float3 moveDir = camRot * inputDir;
		moveDir.y = 0.0f;
		moveDir = math::normalize(moveDir);

		camFwd.y = 0;
		camFwd = math::normalize(camFwd);
		tEnemy->FaceTo(camFwd);

		speed += dT * kAccelerate;

		if (!isDashing)
		{
			if (speed > walkSpeed)
				speed = walkSpeed;
		}
		else
		{
			if (speed > dashSpeed)
				speed = dashSpeed;
		}

		if (speed > kMaxSpeed)
			speed = kMaxSpeed;

		tEnemy->Translate(moveDir * dT * speed);

		aEnemy->CrossFade(1, 0.3f);
	}
	else
	{
		speed -= dT * kDeaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		tEnemy->Translate(tEnemy->GetForwardVector() * dT * speed);

		aEnemy->CrossFade(0, 0.2f);
	}
}

//-------------------------------------------------------------------------------------------------
// Enemy Actions

// Transistion to Aiming Mode
void Enemy::Aim()
{
	// TODO
}

// Attack (Uses a combo system)
void Enemy::Attack()
{
	// TODO
}

// Dash forward (move faster)
void Enemy::Dash()
{
	// TODO
	// Set dashing to true if cool allows
	isDashing = true;
}

// Dodge, in the current Enemy direction
void Enemy::Dodge()
{
	// TODO
}

// Interact with interactable object
void Enemy::Interact()
{
	// TODO
}

// Combo Special move (Sword/Gun)
void Enemy::Special()
{
	// TODO
}

// Transistion to Vision Hack Mode
void Enemy::VisionHack()
{
	// TODO
}

//-------------------------------------------------------------------------------------------------
// Setters



//-------------------------------------------------------------------------------------------------
// Getters

// Is the Enemy in air
bool Enemy::IsInAir()
{
	return inAir;
}

// Return Enemy Position
tofu::math::float3 Enemy::GetPosition()
{
	return tEnemy->GetLocalPosition();
}

// Return Enemy Forward
tofu::math::float3 Enemy::GetForward()
{
	return tEnemy->GetForwardVector();
}