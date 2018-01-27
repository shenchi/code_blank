#include "SceneManager.h"

#include "FileIO.h"
#include "MemoryAllocator.h"
#include "RenderingSystem.h"
#include "RenderingComponent.h"
#include "PhysicsComponent.h"

namespace
{
	typedef const rapidjson::Value& value_t;
}

namespace tofu
{
	int32_t SceneManager::Init()
	{
		CHECKED(resMgr.Init());
		return kOK;
	}

	int32_t SceneManager::LoadScene(const char * filename)
	{
		rapidjson::Document d;
		char* json = nullptr;
		CHECKED(FileIO::ReadFile(
			filename,
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

		return kOK;
	}

	int32_t SceneManager::LoadSceneEntity(const rapidjson::Value& value, tofu::TransformComponent parent)
	{
		if (!value.HasMember("transform"))
		{
			return kErrUnknown;
		}

		value_t transform = value["transform"];
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
			rotate["w"].GetFloat(),
			rotate["x"].GetFloat(),
			rotate["y"].GetFloat(),
			rotate["z"].GetFloat()
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

		if (value.HasMember("components"))
		{
			CHECKED(LoadComponents(value["components"], e));
		}

		if (value.HasMember("children") && value["children"].IsArray())
		{
			value_t children = value["children"];
			for (rapidjson::SizeType i = 0; i < children.Size(); i++)
			{
				CHECKED(LoadSceneEntity(children[i], t));
			}
		}

		return kOK;
	}

	int32_t SceneManager::LoadComponents(const rapidjson::Value& value, tofu::Entity e)
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
}
