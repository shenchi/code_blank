#include "SceneLoadingDemo.h"

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

int32_t SceneLoadingDemo::Init()
{
	CHECKED(resMgr.LoadConfig());

	{
		void* data1 = MemoryAllocator::Allocators[kAllocLevelBasedMem].Allocate(16, 4);
		void* data2 = MemoryAllocator::Allocators[kAllocLevelBasedMem].Allocate(16, 4);
		if (nullptr == data1 || nullptr == data2)
		{
			return kErrUnknown;
		}

		*reinterpret_cast<math::float4*>(data1) = math::float4{ 1, 1, 1, 1 };
		*reinterpret_cast<math::float4*>(data2) = math::float4{ 0, 0, 1, 1 };

		TextureHandle defaultAlbedoTex = RenderingSystem::instance()->CreateTexture(kFormatR8g8b8a8Unorm, 1, 1, 16, data1);
		if (!defaultAlbedoTex)
			return kErrUnknown;

		TextureHandle defaultNormalMap = RenderingSystem::instance()->CreateTexture(kFormatR8g8b8a8Unorm, 1, 1, 16, data2);
		if (!defaultNormalMap)
			return kErrUnknown;

		resMgr.SetDefaultAlbedoMap(defaultAlbedoTex);
		resMgr.SetDefaultNormalMap(defaultNormalMap);
	}
	//

	rapidjson::Document d;
	char* json = nullptr;
	CHECKED(FileIO::ReadFile(
		"assets/scenes/test.json",
		true,
		4,
		kAllocLevelBasedMem,
		reinterpret_cast<void**>(&json),
		nullptr));

	d.Parse(json);

	if (!d.HasMember("entities"))
	{
		return kErrUnknown;
	}

	const auto& entities = d["entities"];
	if (!entities.IsArray())
	{
		return kErrUnknown;
	}

	for (rapidjson::SizeType i = 0; i < entities.Size(); i++)
	{
		CHECKED(LoadSceneEntity(entities[i], TransformComponent()));
	}

	{
		Entity e = Entity::Create();

		tPlayer = e.AddComponent<TransformComponent>();
		tPlayer->SetLocalPosition(math::float3{ -5.0f, 8.0f, -5.0f });
		//tPlayer->SetLocalPosition(math::float3{ 0.0f, 8.0f, 0.0f });
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
		pPlayer->SetCapsuleCollider(50.0f, 100.0f);
		pPlayer->SetColliderOrigin(math::float3{ 0.0f, 1.0f, 0.0f });
	}

	// camera
	{
		Entity e = Entity::Create();

		tCamera = e.AddComponent<TransformComponent>();

		cam = e.AddComponent<CameraComponent>();

		cam->SetFOV(60.0f);
		tCamera->SetLocalPosition(math::float3{ 0, 0, -2 });

		Material* skyboxMat = RenderingSystem::instance()->CreateMaterial(MaterialType::kMaterialTypeSkybox);
		TextureHandle tex = RenderingSystem::instance()->CreateTexture("assets/craterlake.texture");
		skyboxMat->SetTexture(tex);

		cam->SetSkybox(skyboxMat);
	}

	pitch = InitPitch;
	yaw = 0.0f;

	return kOK;
}

int32_t SceneLoadingDemo::Shutdown()
{
	return kOK;
}

int32_t SceneLoadingDemo::Update()
{
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

	return kOK;
}

int32_t SceneLoadingDemo::LoadSceneEntity(const rapidjson::Value& v, TransformComponent parent)
{
	if (!v.HasMember("transform"))
	{
		return kErrUnknown;
	}

	value_t transform = v["transform"];
	value_t translate = transform["translate"];
	value_t rotate = transform["rotate"];
	value_t scale = transform["scale"];

	Entity e = Entity::Create();
	auto t = e.AddComponent<TransformComponent>();

	t->SetLocalPosition(math::float3
		{
			translate["x"].GetFloat(),
			translate["y"].GetFloat(),
			translate["z"].GetFloat()
		});

	t->SetLocalRotation(math::quat
		{
			rotate["x"].GetFloat(),
			rotate["y"].GetFloat(),
			rotate["z"].GetFloat(),
			rotate["w"].GetFloat()
		});

	t->SetLocalScale(math::float3
		{
			scale["x"].GetFloat(),
			scale["y"].GetFloat(),
			scale["z"].GetFloat()
		});

	if (parent)
	{
		t->SetParent(parent);
	}

	if (v.HasMember("components"))
	{
		CHECKED(LoadComponents(v["components"], e));
	}

	if (v.HasMember("children") && v["children"].IsArray())
	{
		value_t children = v["children"];
		for (rapidjson::SizeType i = 0; i < children.Size(); i++)
		{
			CHECKED(LoadSceneEntity(children[i], t));
		}
	}

	return kOK;
}

int32_t SceneLoadingDemo::LoadComponents(const rapidjson::Value & value, tofu::Entity e)
{
	if (!e)
	{
		return kErrUnknown;
	}

	if (!value.IsArray() || value.Size() == 0)
	{
		return kOK;
	}

	char path[1024] = "assets/";
	size_t origLen = strlen(path);

	for (rapidjson::SizeType i = 0; i < value.Size(); i++)
	{
		value_t comp = value[i];
		const char* typeStr = comp["type"].GetString();
		if (strcmp(typeStr, "renderable") == 0)
		{
			// model
			path[origLen] = 0;
			strcat_s(path, 1024, comp["model"].GetString());
			Model* model = RenderingSystem::instance()->CreateModel(path);

			if (nullptr == model)
			{
				return kErrUnknown;
			}

			// rendering component
			auto r = e.AddComponent<RenderingComponent>();
			r->SetModel(model);

			// material
			value_t matList = comp["materials"];
			for (rapidjson::SizeType i = 0; i < matList.Size(); i++)
			{
				Material* material = resMgr.LoadMaterial(matList[i].GetString());
				if (nullptr == material)
				{
					return kErrUnknown;
				}
				r->AddMaterial(material);
			}
			assert(r->GetNumMaterial() != 0);
		}
		else if (strcmp(typeStr, "physics") == 0)
		{
			auto p = e.AddComponent<PhysicsComponent>();

			// TODO
			p->SetStatic(true);

			const char* colliderTypeStr = comp["colliderType"].GetString();
			if (strcmp(colliderTypeStr, "mesh") == 0)
			{
				path[origLen] = 0;
				strcat_s(path, 1024, comp["model"].GetString());
				Model* model = RenderingSystem::instance()->CreateModel(path);

				p->SetMeshCollider(model);
			}
			else if (strcmp(colliderTypeStr, "box") == 0)
			{
				value_t size = comp["size"];
				p->SetBoxCollider(math::float3{
					size["x"].GetFloat(),
					size["y"].GetFloat(),
					size["z"].GetFloat()
					});

				value_t center = comp["center"];
				p->SetColliderOrigin(math::float3{
					center["x"].GetFloat(),
					center["y"].GetFloat(),
					center["z"].GetFloat()
					});
			}
		}
	}

	return kOK;
}
