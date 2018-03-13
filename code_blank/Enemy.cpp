#include "Enemy.h"
#include <PhysicsSystem.h>
#include <RenderingComponent.h>

using namespace tofu;

Enemy::Enemy(CharacterDetails details, void* comp)
{

	CombatManagerDetails combatDetails = {};
	combatDetails.moveDir = 0;
	combatDetails.inCombatDuration = 4.0f;
	combatDetails.maxShotDistance = 20.0f;
	combatDetails.minShotDistance = 2.0f;
	combatDetails.jumpUpTime = 0.5f;
	combatDetails.jumpAirTime = 0.3f;
	combatDetails.jumpDownTime = 0.35f;
	combatDetails.comboTimer = 0.0f;
	combatDetails.maxComboTime = 2.0f;
	combatDetails.dodgeTime = 0.0f;
	combatDetails.rollTime = 0.5f;
	combatDetails.rollSpeed = 3.0f;
	combatDetails.hitTime = 1.0f;
	combatDetails.hitMaxWalkSpeed = 1.0f;
	combatDetails.adjustSpeed = 2.0f;
	combatDetails.adjustMinDistance = 0.75f;
	combatDetails.adjustMaxDistance = 2.5f;
	combatDetails.adjustAgle = 5.0f;

	Init(false, nullptr, combatDetails);

	tag = details.tag;

	{
		Entity e = Entity::Create();

		tEnemy = e.AddComponent<TransformComponent>();
		tEnemy->SetLocalPosition(details.position);
		tEnemy->SetLocalScale(math::float3{ 0.01f, 0.01f, 0.01f });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/archer.model");

		aEnemy = e.AddComponent<AnimationComponent>();

		AnimationStateMachine *stateMachine = aEnemy->GetStateMachine();

		AnimationState *idle = stateMachine->AddState("idle");
		idle->animationName = "idle";
		AnimationState *walk = stateMachine->AddState("walk");
		walk->animationName = "walk";
		AnimationState *jump = stateMachine->AddState("jump");
		jump->animationName = "jump";
		AnimationState *run = stateMachine->AddState("run");
		run->animationName = "run";

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

		SetComponents(tEnemy, pEnemy, aEnemy);
	}

	walkSpeed = 2.0f;
	dashSpeed = 4.0f;

	jumpPower = 4.0f;

	stateTimer = 0;

	physics = tofu::PhysicsSystem::instance();

	//combatManager = new CombatManager(true, comp, this);

	//gEnemy = new GameplayAnimationMachine(aEnemy, combatManager);
	gCharacter->SetAnimComp(aEnemy);

	//rigidbody = GetComponent<Rigidbody>();
	//rigidbody.constraints = RigidbodyConstraints.FreezeRotationX | RigidbodyConstraints.FreezeRotationY | RigidbodyConstraints.FreezeRotationZ;
	origGroundCheckDistance = groundCheckDistance;
	charBodyRotation = tEnemy->GetWorldRotation(); //charBody.transform.rotation;
	rotation = charBodyRotation; //m_Rigidbody.transform.rotation;
	turnMod = 90.0f / 200.0f;
}

Enemy::~Enemy() {}

void Enemy::Update(float dT)
{
	Character::Update(dT);
	//combatManager->Update(dT);
}

void Enemy::UpdateState(float dT)
{

}





// TODO
// Change as needed, this is really only a temp function for testing
// Enemy Movement
void Enemy::MoveEnemy(float dT, bool jump, math::float3 inputDir)
{
	Update(dT);

	if (math::length(inputDir) > 0.25f)
	{
		//math::float3 moveDir = rot * inputDir;
		math::float3 moveDir = inputDir;
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

		//aEnemy->CrossFade(1, 0.3f);
		aEnemy->CrossFade("Walk", 0.3f, 0);
	}
	else
	{
		speed -= dT * kDeaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		tEnemy->Translate(tEnemy->GetForwardVector() * dT * speed);

		aEnemy->CrossFade("Idle", 0.2f, 0);
		//aEnemy->CrossFade(0, 0.2f);
	}

	if (jump && !inAir)
	{
		pEnemy->ApplyImpulse(math::float3{ 0.0f, 2.0f, 0.0f });
	}
}



//-------------------------------------------------------------------------------------------------
// Enemy Actions

// Aim at Player
void Enemy::Aim()
{

}


// Attack (Uses a combo system)
void Enemy::Attack()
{
	// TODO
}

// Kill off the enmey by turning it off and hiding it
void Enemy::Die()
{

}


//-------------------------------------------------------------------------------------------------
// Setters



//-------------------------------------------------------------------------------------------------
// Getters

