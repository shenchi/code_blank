#pragma once

#include "Component.h"
#include "RenderingSystem.h"

namespace tofu
{
	class RenderingSystem;

	class RenderingComponentData
	{
	public:
		RenderingComponentData() : RenderingComponentData(Entity()) {}

		RenderingComponentData(Entity e) 
			: 
			entity(e),
			model(),
			material()
		{}

		void SetModel(ModelHandle handle) { model = handle; }

		void SetMaterial(MaterialHandle handle) { material = handle; }

	private:
		friend class RenderingSystem;

		Entity				entity;
		ModelHandle			model;
		MaterialHandle		material;
	};

	typedef Component<RenderingComponentData> RenderingComponent;
}
