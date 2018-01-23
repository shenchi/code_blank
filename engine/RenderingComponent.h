#pragma once

#include "Component.h"

namespace tofu
{
	class RenderingSystem;
	class Model;
	class Material;
	class Light;

	class RenderingComponentData
	{
		friend class RenderingSystem;

	public:
		RenderingComponentData() : RenderingComponentData(Entity()) {}

		RenderingComponentData(Entity e) 
			: 
			entity(e),
			model(nullptr),
			material(nullptr),
			light(nullptr)
		{}

		void SetModel(Model* model) { this->model = model; }

		void SetMaterial(Material* material) { this->material = material; }

		void SetLight(Light* light) { this->light = light; }

		Model* GetModel() const { return model; }

		Material* GetMaterial() const { return material; }

		Light* GetLight() const { return light; }

	private:
		Entity				entity;
		Model*				model;
		Material*			material;
		Light*              light;
	};

	typedef Component<RenderingComponentData> RenderingComponent;
}
