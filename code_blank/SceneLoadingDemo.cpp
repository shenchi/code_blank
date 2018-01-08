#include "SceneLoadingDemo.h"

using namespace tofu;

typedef const rapidjson::Value& value_t;

int32_t SceneLoadingDemo::Init()
{
	CHECKED(resMgr.LoadConfig());

	rapidjson::Document d;
	char* json = nullptr;
	CHECKED(FileIO::ReadFile(
		"assets/scenes/test2.json", 
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
		tCamera->Translate(tCamera->GetForwardVector() * Time::DeltaTime);
	}
	else if (input->IsButtonDown(kKeyS))
	{
		tCamera->Translate(-tCamera->GetForwardVector() * Time::DeltaTime);
	}

	if (input->IsButtonDown(kKeyD))
	{
		tCamera->Translate(tCamera->GetRightVector() * Time::DeltaTime);
	}
	else if (input->IsButtonDown(kKeyA))
	{
		tCamera->Translate(-tCamera->GetRightVector() * Time::DeltaTime);
	}

	if (input->IsButtonDown(kKeySpace))
	{
		tCamera->Translate(tCamera->GetUpVector() * Time::DeltaTime);
	}
	else if (input->IsButtonDown(kKeyLeftShift))
	{
		tCamera->Translate(-tCamera->GetUpVector() * Time::DeltaTime);
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
			LoadSceneEntity(children[i], t);
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

			// material
			Material* material = resMgr.LoadMaterial(comp["materials"][0].GetString());
			if (nullptr == material)
			{
				return kErrUnknown;
			}

			// rendering component
			auto r = e.AddComponent<RenderingComponent>();
			r->SetModel(model);
			r->SetMaterial(material);
		}
	}

	return kOK;
}
