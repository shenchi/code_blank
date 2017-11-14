#pragma once

#include "Component.h"

namespace tofu
{
	class RenderingSystem;
	class Model;
	class Material;
	class AnimationState;

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
			animState(nullptr)
		{}

		void SetModel(Model* model) { this->model = model; }

		void SetMaterial(Material* material) { this->material = material; }

	private:
		Entity				entity;
		Model*				model;
		Material*			material;
		AnimationState*		animState;

	};

	typedef Component<RenderingComponentData> RenderingComponent;
}
