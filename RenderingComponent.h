#pragma once

#include "Component.h"
#include "RenderingSystem.h"

namespace tofu
{

	class RenderingComponent : public Component<RenderingComponent>
	{
	public:
		void SetMesh(MeshHandle handle) { mesh = handle; }
		void SetMaterial(MaterialHandle handle) { material = handle; }

	private:
		MeshHandle			mesh;
		MaterialHandle		material;
	};

}
