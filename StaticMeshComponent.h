#pragma once

#include "Component.h"
#include "RenderingSystem.h"

namespace tofu
{
	class RenderingSystem;

	class StaticMeshComponentData
	{
	public:
		StaticMeshComponentData() : StaticMeshComponentData(Entity()) {}

		StaticMeshComponentData(Entity e) 
			: 
			entity(e),
			model(),
			material()
		{}

		void SetModel(Model* m) { model = m; }

		void SetMaterial(Material* mat) { material = mat; }

	private:
		friend class RenderingSystem;

		Entity				entity;
		Model*				model;
		Material*			material;
	};

	typedef Component<StaticMeshComponentData> StaticMeshComponent;
}
