#pragma once

#include "Common.h"
#include "Entity.h"

#include <cassert>

namespace tofu
{
	template<class T>
	class Component
	{
	protected:
		Entity		entity;
	
	public:
		Component() : entity(Entity::Invalid) {}

		Component(Entity e) : entity(e) {}

		operator bool() const
		{
			return entity.id < numComponents; // TODO
		}

		T* operator -> () const
		{
			assert(false && "This implementation is wrong!");
			assert(true == *this);
			return &components[entity.id]; // TODO !!!!
		}

	public:
		static Component<T> Create(Entity e)
		{
			if (numComponents >= MAX_COMPONENTS)
				return Component<T>();

			uint32_t loc = numComponents;
			numComponents++;

			components[loc] = T();
			T.entity = e;

			return Component<T> {e};
		}

	protected:
		static T components[MAX_ENTITIES];
		static uint32_t numComponents;
	};

	template<class T>
	T Component<T>::components[MAX_ENTITIES] = {};

	template<class T>
	uint32_t Component<T>::numComponents = 0;
}
