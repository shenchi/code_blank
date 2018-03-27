#include "SceneManager.h"

#include "FileIO.h"
#include "MemoryAllocator.h"
#include "RenderingSystem.h"
#include "RenderingComponent.h"
#include "PhysicsComponent.h"
#include "LightComponent.h"

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
		int32_t count = 0;
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

		// Entities
		if (!d.HasMember("entities"))
		{
			return kErrUnknown;
		}

		const auto& entities = d["entities"];
		if (!entities.IsArray())
		{
			return kErrUnknown;
		}

		// Path Nodes
		if (!d.HasMember("pathNodes"))
		{
			return kErrUnknown;
		}

		const auto& pathNodeList = d["pathNodes"];
		if (!pathNodeList.IsArray())
		{
			return kErrUnknown;
		}

		// Spawn Nodes
		if (!d.HasMember("spawnerNodes"))
		{
			return kErrUnknown;
		}

		const auto& spawnerNodes = d["spawnerNodes"];
		if (!spawnerNodes.IsArray())
		{
			return kErrUnknown;
		}

		// Trigger Nodes
		if (!d.HasMember("triggerNodes"))
		{
			return kErrUnknown;
		}

		const auto& triggerNodes = d["triggerNodes"];
		if (!triggerNodes.IsArray())
		{
			return kErrUnknown;
		}

		// load entities
		for (rapidjson::SizeType i = 0; i < entities.Size(); i++)
		{
			CHECKED(LoadSceneEntity(entities[i], TransformComponent()));
		}

		// add path nodes to a vector
		pathNodes = new std::vector<PathNode*>(pathNodeList.Size());
		for (rapidjson::SizeType i = 0; i < pathNodeList.Size(); i++)
		{
			CHECKED(AddPathNode(pathNodeList[i], count));
			count++;
		}

		return kOK;
	}

	int32_t SceneManager::AddPathNode(const rapidjson::Value& value, int32_t i)
	{
		if (!value.HasMember("name") && !value.HasMember("position"))
		{
			return kErrUnknown;
		}
		PathNode* temp = new PathNode();
		temp->name = value["name"].GetString();
		temp->position.x = value["position"]["x"].GetFloat();
		temp->position.y = value["position"]["y"].GetFloat();
		temp->position.z = value["position"]["z"].GetFloat();
		PathNode* temp_1 = new PathNode();
		temp_1->name = value["nearby_1"].GetString();
		PathNode* temp_2 = new PathNode();
		temp_2->name = value["nearby_2"].GetString();
		PathNode* temp_3 = new PathNode();
		temp_3->name = value["nearby_3"].GetString();
		PathNode* temp_4 = new PathNode();
		temp_4->name = value["nearby_4"].GetString();

		temp->nearby_1 = temp_1;
		temp->nearby_2 = temp_2;
		temp->nearby_3 = temp_3;
		temp->nearby_4 = temp_4;
		pathNodes->at(i) = temp;
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
			else if (strcmp(typeStr, "light") == 0)
			{
				auto l = e.AddComponent<LightComponent>();
				const char* lightTypeStr = comp["lightType"].GetString();
				if (strcmp(lightTypeStr, "directional") == 0)
				{
					l->SetType(LightType::kLightTypeDirectional);
				}
				else if (strcmp(lightTypeStr, "point") == 0)
				{
					l->SetType(LightType::kLightTypePoint);
				}
				else if (strcmp(lightTypeStr, "spot") == 0)
				{
					l->SetType(LightType::kLightTypeSpot);
				}
				value_t color = comp["color"];
				l->SetColor(math::float4{
					color["x"].GetFloat(),
					color["y"].GetFloat(),
					color["z"].GetFloat(),
					color["w"].GetFloat()
				});
				l->SetRange(comp["range"].GetFloat());
				l->SetIntensity(comp["intensity"].GetFloat());
				l->SetSpotAngle(comp["spotAngle"].GetFloat());
				l->SetCastShadow(comp["castShadow"].GetBool());
			}
		}

		return kOK;
	}

	std::vector<PathNode*>* SceneManager::GetPathNodes()
	{
		return pathNodes;
	}

	int32_t SceneManager::GetPathNodesLength()
	{
		return pathNodes->size();
	}
}
