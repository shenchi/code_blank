#include <Engine.h>
#include "Game.h"

#include <RenderingSystem.h>
#include <InputSystem.h>

using namespace tofu;

InputSystem* input;

namespace
{
	constexpr float MaxPitch = math::PI * 0.25f;
	constexpr float MinPitch = 0.0f;
	constexpr float InitPitch = math::PI * 0.125f;

	constexpr float Accelerate = 6.67f;
	constexpr float Deaccelerate = 10.0f;
	constexpr float WalkSpeed = 2.0f;
}

// Intialization of Game components
int32_t Game::Init()
{
	// Create a camera
	{
		Entity e = Entity::Create();

		tCamera = e.AddComponent<TransformComponent>();

		cam = e.AddComponent<CameraComponent>();

		cam->SetFOV(60.0f);
		tCamera->SetLocalPosition(math::float3{ 0, 0, -2 });
	}

	input = InputSystem::instance();

	currentScene = level;

	// Set up first scene - Introduction
	LoadScene(currentScene);

	return kOK;
}

// Exit the Game
int32_t Game::Shutdown()
{
	return kOK;
}

// Main upate loop
int32_t Game::Update()
{
	uint32_t ret;

	// First check if I should pause
	if (input->IsButtonDown(ButtonId::kKeyEscape))
	{
		// For testing this exits instead of pausing
		Engine::instance()->Quit();
	}

	// Switch for game state
	// Will be overhauled to use JSON file names and possibley multi-threading
	switch (currentScene)
	{
	case 0:	// Intro
	{
		if (input->IsButtonDown(ButtonId::kKeyEnter))
		{
			// Load next scene
			currentScene = menu;
			ret = LoadScene(menu);
			assert(ret);

			// Unload last scene
			lastScene = intro;
			ret = UnloadScene(intro);
			assert(ret);
		}
		break;
	}
	case 1:	// Menu
	{
		if (input->IsButtonDown(ButtonId::kKeyEnter))
		{
			// Load next scene
			currentScene = loading;
			ret = LoadScene(loading);
			assert(ret);

			// Unload last scene
			lastScene = menu;
			ret = UnloadScene(menu);
			assert(ret);
		}
		break;
	}
	case 4:	// Loading
	{
		if (input->IsButtonDown(ButtonId::kKeyEnter))
		{
			// Load next scene
			currentScene = tutorial;
			ret = LoadScene(tutorial);
			assert(ret);

			// Unload last scene
			lastScene = loading;
			ret = UnloadScene(loading);
			assert(ret);
		}
		break;
	}
	case 5:	// Tutorial
	{
		if (input->IsButtonDown(ButtonId::kKeyEnter))
		{
			// Load next scene
			currentScene = level;
			ret = LoadScene(level);
			assert(ret);

			// Unload last scene
			lastScene = tutorial;
			ret = UnloadScene(tutorial);
			assert(ret);
		}
		break;
	}
	case 6:	// Level
	{
		//if (input->IsButtonDown(ButtonId::kKeyEnter))
		//{
		//	// Load next scene
		//	currentScene = credits;
		//	ret = LoadScene(credits);
		//	assert(ret);

		//	// Unload last scene
		//	lastScene = level;
		//	ret = UnloadScene(level);
		//	assert(ret);
		//}

		inAir = !pPlayer->IsCollided();

		InputSystem* input = InputSystem::instance();
		if (input->IsButtonDown(ButtonId::kKeyEscape))
		{
			Engine::instance()->Quit();
		}

		constexpr float sensitive = 0.01f;


		math::float3 inputDir = math::float3();

		if (input->IsGamepadConnected())
		{
			if (input->IsButtonDown(ButtonId::kGamepadFaceRight))
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


		if (input->IsButtonDown(kKeyW))
		{
			inputDir.z = 1.0f;
		}
		else if (input->IsButtonDown(kKeyS))
		{
			inputDir.z = -1.0f;
		}

		if (input->IsButtonDown(kKeyD))
		{
			inputDir.x = 1.0f;
		}
		else if (input->IsButtonDown(kKeyA))
		{
			inputDir.x = -1.0f;
		}

		bool jump = input->IsButtonDown(ButtonId::kKeySpace)
			|| input->IsButtonDown(ButtonId::kGamepadFaceDown);

		math::quat camRot = math::euler(pitch, yaw, 0.0f);
		math::float3 camTgt = tPlayer->GetLocalPosition() + math::float3{ 0.0f, 2.0f, 0.0f };
		math::float3 camPos = camTgt + camRot * (math::float3{ 0.0f, 0.0f, -5.0f });

		tCamera->SetLocalPosition(camPos);
		tCamera->SetLocalRotation(camRot);

		float maxSpeed = WalkSpeed;

		if (math::length(inputDir) > 0.25f)
		{
			math::float3 moveDir = camRot * inputDir;
			moveDir.y = 0.0f;
			moveDir = math::normalize(moveDir);
			tPlayer->FaceTo(moveDir);

			speed += Time::DeltaTime * Accelerate;
			if (speed > maxSpeed)
				speed = maxSpeed;

			tPlayer->Translate(moveDir * Time::DeltaTime * speed);

			anim->CrossFade(1, 0.3f);
		}
		else
		{
			speed -= Time::DeltaTime * Deaccelerate;
			if (speed < 0.0f) speed = 0.0f;
			tPlayer->Translate(tPlayer->GetForwardVector() * Time::DeltaTime * speed);

			anim->CrossFade(0, 0.2f);
		}

		if (jump && !inAir)
		{
			pPlayer->ApplyImpulse(math::float3{ 0.0f, 2.0f, 0.0f });
		}
		break;
	}
	case 10: // Credits
	{
		if (input->IsButtonDown(ButtonId::kKeyEnter))
		{
			// Load next scene
			currentScene = menu;
			ret = LoadScene(menu);
			assert(ret);

			// Unload last scene
			lastScene = credits;
			ret = UnloadScene(credits);
			assert(ret);
		}
		break;
	}
	default:
		assert(true);
		break;
	}


	return kOK;
}

// Load a scene
// Will load from JSON files later
bool Game::LoadScene(uint32_t num)
{
	// Temp code for testing
	switch (num)
	{
		case 0:
		{
			{
				Entity e = Entity::Create();

				tIntro = e.AddComponent<TransformComponent>();

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* quadModel = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* quadMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");

				quadMat->SetTexture(diffuse);

				r->SetMaterial(quadMat);
				r->SetModel(quadModel);

				tIntro->SetLocalPosition(math::float3{ 0.0f, 0.0f, 0.0f });
				cam->SetClearColor(math::float4{ 0.0f, 0.0f, 0.1f, 1.0f });
			}
			break;
		}
		case 1:
		{
			{
				Entity e = Entity::Create();

				tCube = e.AddComponent<TransformComponent>();

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* quadModel = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* quadMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");

				quadMat->SetTexture(diffuse);

				r->SetMaterial(quadMat);
				r->SetModel(quadModel);

				tCube->SetLocalPosition(math::float3{ 0.0f, 0.0f, 0.0f });
			}
			{
				Entity e = Entity::Create();

				tBox = e.AddComponent<TransformComponent>();
				tBox->SetLocalPosition(math::float3{ 0, 10, 10 });

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* model = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
				TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

				material->SetTexture(diffuse);
				material->SetNormalMap(normalMap);

				r->SetMaterial(material);
				r->SetModel(model);

				tBox->SetLocalPosition(math::float3{0.0f, 1.0f, 0.0f});
				//PhysicsComponent ph = e.AddComponent<PhysicsComponent>();
			}
			cam->SetClearColor(math::float4{ 0.0f, 0.1f, 0.0f, 1.0f });
			break;
		}
		case 2:
		{
			break;
		}
		case 3:
		{
			break;
		}
		case 4:
		{
			{
				Entity e = Entity::Create();

				tBox2 = e.AddComponent<TransformComponent>();

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* quadModel = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* quadMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");

				quadMat->SetTexture(diffuse);

				r->SetMaterial(quadMat);
				r->SetModel(quadModel);

				tBox2->SetLocalPosition(math::float3{ -2.0f, 1.0f, 0.0f });
			}
			{
				Entity e = Entity::Create();

				tBox3 = e.AddComponent<TransformComponent>();
				tBox3->SetLocalPosition(math::float3{ 0, 10, 10 });

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* model = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
				TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

				material->SetTexture(diffuse);
				material->SetNormalMap(normalMap);

				r->SetMaterial(material);
				r->SetModel(model);

				tBox3->SetLocalPosition(math::float3{ 2.0f, 1.0f, 0.0f });
				//PhysicsComponent ph = e.AddComponent<PhysicsComponent>();
			}
			cam->SetClearColor(math::float4{ 0.1f, 0.0f, 0.1f, 1.0f });
			break;
		}
		case 5:
		{
			{
				Entity e = Entity::Create();

				tBox4 = e.AddComponent<TransformComponent>();

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* quadModel = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* quadMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");

				quadMat->SetTexture(diffuse);

				r->SetMaterial(quadMat);
				r->SetModel(quadModel);

				tBox4->SetLocalPosition(math::float3{ -2.0f, 1.0f, 0.0f });
			}
			{
				Entity e = Entity::Create();

				tBox5 = e.AddComponent<TransformComponent>();
				tBox5->SetLocalPosition(math::float3{ 0, 10, 10 });

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* model = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
				TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

				material->SetTexture(diffuse);
				material->SetNormalMap(normalMap);

				r->SetMaterial(material);
				r->SetModel(model);

				tBox5->SetLocalPosition(math::float3{ 2.0f, 1.0f, 0.0f });
				//PhysicsComponent ph = e.AddComponent<PhysicsComponent>();
			}
			{
				Material* skyboxMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeSkybox);
				TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/craterlake.texture");
				skyboxMat->SetTexture(tex);

				cam->SetSkybox(skyboxMat);
				cam->SetClearColor(math::float4{ 0.0f, 0.0f, 0.0f, 1.0f });
			}
			break;
		}
		case 6:
		{
			{
				Material* skyboxMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeSkybox);
				TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/craterlake.texture");
				skyboxMat->SetTexture(tex);

				cam->SetSkybox(skyboxMat);
				cam->SetClearColor(math::float4{ 0.0f, 0.0f, 0.0f, 1.0f });
			}
			{
				Entity e = Entity::Create();

				tBox6 = e.AddComponent<TransformComponent>();

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* quadModel = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* quadMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");

				quadMat->SetTexture(diffuse);

				r->SetMaterial(quadMat);
				r->SetModel(quadModel);

				tBox6->SetLocalPosition(math::float3{ -2.0f, 1.0f, 0.0f });
				PhysicsComponent ph = e.AddComponent<PhysicsComponent>();
			}
			{
				Entity e = Entity::Create();

				tBox7 = e.AddComponent<TransformComponent>();
				tBox7->SetLocalPosition(math::float3{ 0, 10, 10 });

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* model = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
				TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

				material->SetTexture(diffuse);
				material->SetNormalMap(normalMap);

				r->SetMaterial(material);
				r->SetModel(model);

				tBox7->SetLocalPosition(math::float3{ 2.0f, 1.0f, 0.0f });
				PhysicsComponent ph = e.AddComponent<PhysicsComponent>();
			}
			{
				Entity e = Entity::Create();

				tGround = e.AddComponent<TransformComponent>();

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* model = RenderingSystem::instance()->CreateModel("assets/ground.model");

				Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
				TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

				material->SetTexture(diffuse);
				material->SetNormalMap(normalMap);

				r->SetMaterial(material);
				r->SetModel(model);

				PhysicsComponent ph = e.AddComponent<PhysicsComponent>();
				ph->SetStatic(true);
				ph->SetBoxCollider(math::float3{ 25.0f, 0.5f, 25.0f });
				ph->SetColliderOrigin(math::float3{ 0.0f, -0.5f, 0.0f });
			}
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
			break;
		}
		case 7:
		{
			break;
		}
		case 8:
		{
			break;
		}
		case 9:
		{
			break;
		}
		case 10:
		{
			{
				Entity e = Entity::Create();

				tBox2 = e.AddComponent<TransformComponent>();

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* quadModel = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* quadMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");

				quadMat->SetTexture(diffuse);

				r->SetMaterial(quadMat);
				r->SetModel(quadModel);

				tBox2->SetLocalPosition(math::float3{ -2.0f, 1.0f, 0.0f });
			}
			{
				Entity e = Entity::Create();

				tBox3 = e.AddComponent<TransformComponent>();
				tBox3->SetLocalPosition(math::float3{ 0, 10, 10 });

				RenderingComponent r = e.AddComponent<RenderingComponent>();

				Model* model = RenderingSystem::instance()->CreateModel("assets/cube.model");

				Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeOpaque);
				TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/stone_wall.texture");
				TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/stone_wall_normalmap.texture");

				material->SetTexture(diffuse);
				material->SetNormalMap(normalMap);

				r->SetMaterial(material);
				r->SetModel(model);

				tBox3->SetLocalPosition(math::float3{ 2.0f, 1.0f, 0.0f });
				//PhysicsComponent ph = e.AddComponent<PhysicsComponent>();
			}
			cam->SetClearColor(math::float4{ 0.0f, 1.0f, 0.0f, 0.2f});
			break;
		}

		default:
			return false;
			break;
	}// End of Switch

	//return kOK;
	return true;
}

// Unload a scene
bool Game::UnloadScene(uint32_t num)
{
	// Temp code until JSON is implemented
	switch (num)
	{
	case 0:
	{
		//tIntro.Destroy();
		break;
	}
	case 1:
	{
		//tCube.Destroy();
		//tBox.Destroy();
		break;
	}
	case 4:
	{
		//tBox2.Destroy();
		//tBox3.Destroy();
		break;
	}
	case 5:
	{
		//tBox4.Destroy();
		//tBox5.Destroy();
		break;
	}
	case 6:
	{
		//tBox6.Destroy();
		//tBox7.Destroy();
		break;
	}
	case 10:
	{
		//tBox2.Destroy();
		//tBox3.Destroy();
		break;
	}
	default:
		return false;
		break;
	}

	return true;
}
