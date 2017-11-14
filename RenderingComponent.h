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
			model(nullptr),
			material(nullptr)
		{}

		void SetModel(Model* model) { this->model = model; }

		void SetMaterial(Material* material) { this->material = material; }

		Model* GetModel() const { return model; }

		Material* GetMaterial() const { return material; }

	private:
		Entity				entity;
		Model*				model;
		Material*			material;

	};

	typedef Component<RenderingComponentData> RenderingComponent;
}
