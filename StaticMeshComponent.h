#pragma once

#include "Component.h"

namespace tofu
{
	class RenderingSystem;
	class Model;
	class Material;

	class StaticMeshComponentData
	{
		friend class RenderingSystem;

	public:
		StaticMeshComponentData() : StaticMeshComponentData(Entity()) {}

		StaticMeshComponentData(Entity e) 
			: 
			entity(e),
			model(nullptr),
			material(nullptr)
		{}

		void SetModel(Model* model) { this->model = model; }

		void SetMaterial(Material* material) { this->material = material; }

	private:
		Entity				entity;
		Model*				model;
		Material*			material;
	};

	typedef Component<StaticMeshComponentData> StaticMeshComponent;
}
