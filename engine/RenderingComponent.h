#pragma once

#include "Component.h"

namespace tofu
{
	class RenderingSystem;
	class Model;
	class Material;

	class RenderingComponentData
	{
		friend class RenderingSystem;

	public:
		RenderingComponentData() : RenderingComponentData(Entity()) {}

		RenderingComponentData(Entity e) 
			: 
			entity(e),
			numMaterials(0),
			model(nullptr),
			materials()
		{}

		void SetModel(Model* model) { this->model = model; }

		void SetMaterial(Material* material) 
		{ 
			materials[0] = material;
			if (numMaterials < 1)
				numMaterials = 1;
		}

		void SetMaterial(uint32_t index, Material* material)
		{
			if (index < numMaterials)
			{
				materials[index] = material;
			}
		}

		void AddMaterial(Material* material)
		{
			if (numMaterials < kMaxMeshesPerModel)
			{
				materials[numMaterials++] = material;
			}
		}

		Model* GetModel() const { return model; }

		Material* GetMaterial() const 
		{
			if (numMaterials > 0)
				return materials[0];
			return nullptr;
		}

		Material* GetMaterial(uint32_t index) const
		{
			if (index < numMaterials)
				return materials[index];
			return nullptr;
		}

		uint32_t GetNumMaterial() const { return numMaterials; }

	private:
		Entity				entity;
		uint32_t			numMaterials;
		Model*				model;
		Material*			materials[kMaxMeshesPerModel];

	};

	typedef Component<RenderingComponentData> RenderingComponent;
}
