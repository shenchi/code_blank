#pragma once

#include "Component.h"
#include "RenderingSystem.h"

namespace tofu
{
	class RenderingSystem;

	class RenderingComponentData
	{
	public:
		void SetModel(ModelHandle handle) { model = handle; }
		void SetMaterial(MaterialHandle handle) { material = handle; }

	private:
		friend class RenderingSystem;

		ModelHandle			model;
		MaterialHandle		material;
	};

	typedef Component<RenderingComponentData> RenderingComponent;
}
