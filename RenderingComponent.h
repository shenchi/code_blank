#pragma once

#include "Component.h"
#include "RenderingSystem.h"

namespace tofu
{
	class RenderingSystem;

	class RenderingComponent : public Component<RenderingComponent>
	{
	public:
		void SetModel(ModelHandle handle) { model = handle; }
		void SetMaterial(MaterialHandle handle) { material = handle; }

	private:
		friend class RenderingSystem;

		ModelHandle			model;
		MaterialHandle		material;

		static RenderingComponent* GetAllComponents() { return components; }
		static uint32_t GetComponentCount() { return numComponents; }
	};

}
