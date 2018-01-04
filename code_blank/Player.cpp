#include "Player.h"
#include <RenderingComponent.h>

using namespace tofu;

Player::Player()
{
	{
		Entity e = Entity::Create();

		tPlayer = e.AddComponent<TransformComponent>();
		tPlayer->SetLocalPosition(math::float3{ 0.0f, 1.0f, 0.0f });
		tPlayer->SetLocalScale(math::float3{ 0.01f, 0.01f, 0.01f });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/archer.model");

		anim = e.AddComponent<AnimationComponent>();

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaqueSkinned);
		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/archer_0.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/archer_1.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);

		pPlayer = e.AddComponent<PhysicsComponent>();

		pPlayer->LockRotation(true, false, true);
		pPlayer->SetCapsuleCollider(0.5f, 1.0f);
		pPlayer->SetColliderOrigin(math::float3{ 0.0f, 1.0f, 0.0f });
	}

	walkSpeed = 2.0f;
	dashSpeed = 4.0f;
}

Player::~Player(){}

void Player::Update()
{
	inAir = !pPlayer->IsCollided();

	// TODO
	// Handle the reset of dashing here
}

// Player Movement
void Player::MoveReg(float dT, bool jump, math::float3 inputDir, math::quat camRot)
{
	Update();

	if (math::length(inputDir) > 0.25f)
	{
		math::float3 moveDir = camRot * inputDir;
		moveDir.y = 0.0f;
		moveDir = math::normalize(moveDir);
		tPlayer->FaceTo(moveDir);

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

		tPlayer->Translate(moveDir * dT * speed);

		anim->CrossFade(1, 0.3f);
	}
	else
	{
		speed -= dT * kDeaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		tPlayer->Translate(tPlayer->GetForwardVector() * dT * speed);

		anim->CrossFade(0, 0.2f);
	}

	if (jump && !inAir)
	{
		pPlayer->ApplyImpulse(math::float3{ 0.0f, 2.0f, 0.0f });
	}
}

// Player Movement in Aiming Mode
void Player::MoveAim(float dT, tofu::math::float3 inputDir, math::quat camRot, tofu::math::float3 camFwd)
{
	Update();

	if (math::length(inputDir) > 0.25f)
	{
		math::float3 moveDir = camRot * inputDir;
		moveDir.y = 0.0f;
		moveDir = math::normalize(moveDir);

		camFwd.y = 0;
		camFwd = math::normalize(camFwd);
		tPlayer->FaceTo(camFwd);

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

		tPlayer->Translate(moveDir * dT * speed);

		anim->CrossFade(1, 0.3f);
	}
	else
	{
		speed -= dT * kDeaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		tPlayer->Translate(tPlayer->GetForwardVector() * dT * speed);

		anim->CrossFade(0, 0.2f);
	}
}

//-------------------------------------------------------------------------------------------------
// Player Actions

// Transistion to Aiming Mode
void Player::Aim()
{
	// TODO
}

// Attack (Uses a combo system)
void Player::Attack()
{
	// TODO
}

// Dash forward (move faster)
void Player::Dash()
{
	// TODO
	// Set dashing to true if cool allows
	isDashing = true;
}

// Dodge, in the current player direction
void Player::Dodge()
{
	// TODO
}

// Interact with interactable object
void Player::Interact()
{
	// TODO
}

// Combo Special move (Sword/Gun)
void Player::Special()
{
	// TODO
}

// Transistion to Vision Hack Mode
void Player::VisionHack()
{
	// TODO
}

//-------------------------------------------------------------------------------------------------
// Setters



//-------------------------------------------------------------------------------------------------
// Getters

// Is the player in air
bool Player::IsInAir()
{
	return inAir;
}

// Return Player Position
tofu::math::float3 Player::GetPosition()
{
	return tPlayer->GetLocalPosition();
}

// Return Player Forward
tofu::math::float3 Player::GetForward()
{
	return tPlayer->GetForwardVector();
}