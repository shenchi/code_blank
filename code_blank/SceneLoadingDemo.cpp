#include "SceneLoadingDemo.h"

using namespace tofu;

typedef const rapidjson::Value& value_t;

namespace
{
	constexpr float speed = 2.0f;
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

	pitch = 0.0f;
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

	if (input->IsGamepadConnected())
	{
		if (input->IsButtonDown(ButtonId::kGamepadFaceRight))
		{
			Engine::instance()->Quit();
		}

		float lt = input->GetLeftTrigger();
		float rt = input->GetRightTrigger();

		float up = rt - lt;

		tCamera->Translate(Time::DeltaTime * (
			tCamera->GetForwardVector() * -input->GetLeftStickY()
			+ tCamera->GetRightVector() * input->GetLeftStickX()
			+ tCamera->GetUpVector() * up
			));

		pitch += sensitive * input->GetRightStickY();
		yaw += sensitive * input->GetRightStickX();
	}

	pitch += sensitive * input->GetMouseDeltaY();
	yaw += sensitive * input->GetMouseDeltaX();

	tCamera->SetLocalRotation(math::euler(pitch, yaw, 0.0f));

	if (input->IsButtonDown(kKeyW))
	{
		tCamera->Translate(tCamera->GetForwardVector() * Time::DeltaTime * speed);
	}
	else if (input->IsButtonDown(kKeyS))
	{
		tCamera->Translate(-tCamera->GetForwardVector() * Time::DeltaTime * speed);
	}

	if (input->IsButtonDown(kKeyD))
	{
		tCamera->Translate(tCamera->GetRightVector() * Time::DeltaTime * speed);
	}
	else if (input->IsButtonDown(kKeyA))
	{
		tCamera->Translate(-tCamera->GetRightVector() * Time::DeltaTime * speed);
	}

	if (input->IsButtonDown(kKeySpace))
	{
		tCamera->Translate(tCamera->GetUpVector() * Time::DeltaTime * speed);
	}
	else if (input->IsButtonDown(kKeyLeftShift))
	{
		tCamera->Translate(-tCamera->GetUpVector() * Time::DeltaTime * speed);
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
		if (strcmp(comp["type"].GetString(), "renderable") == 0)
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
	}

	return kOK;
}
