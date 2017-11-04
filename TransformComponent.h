#pragma once

#include "Component.h"
#include "Transform.h"
#include <vector>

namespace tofu
{
	class TransformComponentData;

	typedef Component<TransformComponentData> TransformComponent;

	class TransformComponentData
	{
	public:
		TransformComponentData() : TransformComponentData(Entity()) {}

		TransformComponentData(Entity e)
			: 
			entity(e),
			parent(),
			dirty(1)
		{}

		inline void					SetParent(TransformComponent parent);

		inline TransformComponent	GetParent() const { return parent; }

		const Transform&			GetLocalTransform();

		const Transform&			GetWorldTransform();

	public:

		// auxiliary functions

		// TODO

	private:
		Entity							entity;
		TransformComponent				parent;
		std::vector<TransformComponent>	children;

		Transform						localTransform;
		Transform						worldTransform;
		uint32_t						dirty : 1;
	};

}