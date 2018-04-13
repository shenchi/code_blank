#include "RenderingTest.h"

using namespace tofu;

typedef const rapidjson::Value& value_t;

namespace
{
	constexpr float MaxPitch = math::PI * 0.25f;
	constexpr float MinPitch = 0.0f;
	constexpr float InitPitch = math::PI * 0.125f;

	constexpr float Accelerate = 6.67f;
	constexpr float Deaccelerate = 10.0f;
	constexpr float WalkSpeed = 2.0f;
}

int32_t RenderingTest::Init()
{
	CHECKED(sceneMgr.Init());

	CHECKED(sceneMgr.LoadScene("assets/scenes/rendering_test.json"));

	{
		Entity e = Entity::Create();

		tPlayer = e.AddComponent<TransformComponent>();
		tPlayer->SetLocalPosition(math::float3{ 0.0f, 8.0f, 0.0f });
		tPlayer->SetLocalScale(math::float3{ 0.01f, 0.01f, 0.01f });

		RenderingComponent r = e.AddComponent<RenderingComponent>();

		Model* model = RenderingSystem::instance()->CreateModel("assets/archer.model");

		anim = e.AddComponent<AnimationComponent>();

		AnimationStateMachine *stateMachine = anim->GetStateMachine();

		AnimationState *idle = stateMachine->AddState("idle");
		idle->animationName = "idle";
		AnimationState *walk = stateMachine->AddState("walk");
		walk->animationName = "walk";

		Material* material = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialDeferredGeometryOpaqueSkinned);

		TextureHandle diffuse = RenderingSystem::instance()->CreateTexture("assets/archer_0.texture");
		TextureHandle normalMap = RenderingSystem::instance()->CreateTexture("assets/archer_1.texture");

		material->SetTexture(diffuse);
		material->SetNormalMap(normalMap);

		r->SetMaterial(material);
		r->SetModel(model);

		pPlayer = e.AddComponent<PhysicsComponent>();

		pPlayer->LockRotation(true, true, true);
		pPlayer->SetCapsuleCollider(50.0f, 100.0f);
		pPlayer->SetColliderOrigin(math::float3{ 0.0f, 100.0f, 0.0f });
	}

	// camera
	{
		Entity e = Entity::Create();

		tCamera = e.AddComponent<TransformComponent>();

		cam = e.AddComponent<CameraComponent>();

		cam->SetFOV(60.0f);
		tCamera->SetLocalPosition(math::float3{ 0, 0, -2 });

		TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/nightSky.dds");
		//TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/textures/test/darkcity - Copy.texture");
		//TextureHandle skyboxDiff = RenderingSystem::instance()->CreateTexture("assets/textures/test/diffuseIrradianceMapd - Copy.texture");
		//TextureHandle skyboxSpec = RenderingSystem::instance()->CreateTexture("assets/textures/test/prefilteredMapd - Copy.texture");

		cam->SetSkybox(tex);
		//cam->SetSkyboxDiffuseMap(skyboxDiff);
		//cam->SetSkyboxSpecularMap(skyboxSpec);
	}

	{
		uiTex = RenderingSystem::instance()->CreateTexture("assets/001.texture", kResourceGlobal);
		uiTex1 = RenderingSystem::instance()->CreateTexture("assets/ui.texture", kResourceGlobal);

		CHECKED(atlas.LoadFromFile("assets/ui.json"));

		if (!uiTex || !uiTex1)
		{
			return kErrUnknown;
		}

		mainMenuFocused = true;
	}

	pitch = InitPitch;
	yaw = 0.0f;

	sceneLoaded = true;

	return kOK;
}

int32_t RenderingTest::Shutdown()
{
	return kOK;
}

int32_t RenderingTest::Update()
{
	InputSystem* input = InputSystem::instance();
	if (input->IsButtonDown(ButtonId::kKeyEscape))
	{
		Engine::instance()->Quit();
	}

	if (input->IsButtonDown(ButtonId::kKeyL) && !sceneLoaded)
	{
		Init();
		return kOK;
	}

	if (!sceneLoaded) return kOK;

	if (input->IsButtonDown(ButtonId::kKeyU))
	{
		sceneMgr = SceneManager();
		CHECKED(Engine::instance()->UnloadLevel());
		sceneLoaded = false;
		return kOK;
	}

	constexpr float sensitive = 0.01f;


	math::float3 inputDir = math::float3();

	if (input->IsGamepadConnected())
	{
		if (input->IsButtonDown(ButtonId::kGamepadFaceRight))
		{
			Engine::instance()->Quit();
		}

		inputDir.z = input->GetLeftStickY();
		inputDir.x = input->GetLeftStickX();

		pitch -= sensitive * input->GetRightStickY();
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
		tPlayer->FaceTo(-moveDir);

		speed += Time::DeltaTime * Accelerate;
		if (speed > maxSpeed)
			speed = maxSpeed;

		tPlayer->Translate(moveDir * Time::DeltaTime * speed);

		anim->CrossFade("walk", 0.3f, 0);
	}
	else
	{
		speed -= Time::DeltaTime * Deaccelerate;
		if (speed < 0.0f) speed = 0.0f;
		tPlayer->Translate(tPlayer->GetForwardVector() * Time::DeltaTime * speed);

		anim->CrossFade("idle", 0.1f);
	}

	{
		GUI* gui = GUI::instance();
		gui->SetCanvasSize(1920, 1080);

		gui->SetupLayer(0, uiTex);
		gui->Texture(0, -960, -540, 1920, 1080);

		gui->Text(0, 0, -400, 128, "Code Blank", math::float4(0, 1, 1, 1), kTextAlignCenter | kTextAlignTop);

		gui->SetupLayer(1, uiTex1);

		GUIStyle style1 = {
			{ 1, 1, 1, 0.5f },
			{ 1, 1, 1, 1 },
			atlas.rects[0],
			atlas.rects[0],
		};
		GUIStyle style2 = {
			{ 1, 1, 1, 0.5f },
			{ 1, 1, 1, 1 },
			atlas.rects[1],
			atlas.rects[1],
		};
		GUIStyle style3 = {
			{ 1, 1, 1, 0.5f },
			{ 1, 1, 1, 1 },
			atlas.rects[5],
			atlas.rects[5],
		};
		GUIStyle style4 = {
			{ 1, 1, 1, 0.5f },
			{ 1, 1, 1, 1 },
			atlas.rects[2],
			atlas.rects[2],
		};
		GUIStyle style5 = {
			{ 1, 1, 1, 0.5f },
			{ 1, 1, 1, 1 },
			atlas.rects[4],
			atlas.rects[4],
		};


		GUIStyle style6 = {
			{ 1, 1, 1, 0.5f },
			{ 1, 1, 1, 1 },
			atlas.rects[3],
			atlas.rects[3],
		};
		GUIStyle style7 = {
			{ 1, 1, 1, 0.5f },
			{ 1, 1, 1, 1 },
			atlas.rects[7],
			atlas.rects[7],
		};
		GUIStyle style8 = {
			{ 1, 1, 1, 0.5f },
			{ 1, 1, 1, 1 },
			atlas.rects[6],
			atlas.rects[6],
		};

		if (!optionMenuFocused)
		{
			// main menu
			gui->BeginMenu(mainMenuSelectedItem, mainMenuFocused);

			gui->BeginMenuItem();
			gui->Image(1, -500, 0, 288, 53, style1);
			gui->EndMenuItem();

			gui->BeginMenuItem();
			gui->Image(1, -500, 60, 288, 53, style2);
			gui->EndMenuItem();

			gui->BeginMenuItem();
			gui->Image(1, -500, 120, 288, 53, style3);
			gui->EndMenuItem();

			gui->BeginMenuItem();
			gui->Image(1, -500, 180, 288, 53, style4);
			gui->EndMenuItem();

			gui->BeginMenuItem();
			gui->Image(1, -500, 240, 288, 53, style5);
			gui->EndMenuItem();

			mainMenuSelectedItem = gui->EndMenu();

			if (levelMenuFocused)
			{
				gui->BeginMenu(levelMenuSelectedItem, levelMenuFocused);

				gui->BeginMenuItem();
				gui->Image(1, -144, 0, 288, 53, style6);
				gui->EndMenuItem();

				gui->BeginMenuItem();
				gui->Image(1, -144, 60, 288, 53, style7);
				gui->EndMenuItem();

				gui->BeginMenuItem();
				gui->Image(1, -144, 120, 288, 53, style8);
				gui->EndMenuItem();

				levelMenuSelectedItem = gui->EndMenu();
			}
		}
		else
		{
			gui->BeginMenu(optionMenuSelectedItem, optionMenuFocused);

			gui->BeginMenuItem();
			gui->Label(1, -400, -100, 400, 50, 36, "Inverse Camera Axis X", style1, kTextAlignLeft | kTextAlignMiddle);

			gui->BeginSwitch(300, -100, 100, 50, inverseCameraAxisX);
			gui->Option(1, 36, "Off", style1);
			gui->Option(1, 36, "On", style1);
			inverseCameraAxisX = gui->EndSwitch();

			gui->EndMenuItem();

			gui->BeginMenuItem();
			gui->Label(1, -400, -50, 400, 50, 36, "Inverse Camera Axis Y", style1, kTextAlignLeft | kTextAlignMiddle);

			gui->BeginSwitch(300, -50, 100, 50, inverseCameraAxisY);
			gui->Option(1, 36, "Off", style1);
			gui->Option(1, 36, "On", style1);
			inverseCameraAxisY = gui->EndSwitch();

			gui->EndMenuItem();

			gui->BeginMenuItem();
			gui->Image(1, -144, 120, 288, 53, style8);
			gui->EndMenuItem();

			optionMenuSelectedItem = gui->EndMenu();
		}

		if (input->IsButtonReleased(kKeyEnter) || input->IsButtonReleased(kGamepadFaceDown))
		{
			if (mainMenuFocused)
			{
				switch (mainMenuSelectedItem)
				{
				case 0:
					mainMenuFocused = false;
					levelMenuFocused = true;
					levelMenuSelectedItem = 0;
					break;
				case 1:
					mainMenuFocused = false;
					optionMenuFocused = true;
					optionMenuSelectedItem = 0;
					break;
				case 4:
					Engine::instance()->Quit();
					break;
				default:
					break;
				}
			}
			else if (levelMenuFocused)
			{
				switch (levelMenuSelectedItem)
				{
				case 2:
					mainMenuFocused = true;
					levelMenuFocused = false;
					break;
				default:
					break;
				}
			}
			else if (optionMenuFocused)
			{
				switch (optionMenuSelectedItem)
				{
				case 2:
					mainMenuFocused = true;
					optionMenuFocused = false;
					break;
				default:
					break;
				}
			}
		}
	}

	return kOK;
}
