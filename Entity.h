#pragma once

#include "common.h"
#include "HandleAllocator.h"
#include "Component.h"

namespace tofu
{
	template<class T>
	class Component;

	class Entity
	{
	public:
		uint32_t	id;

		Entity(uint32_t _id = MAX_ENTITIES) : id(_id) {}

		inline operator bool() const { return entityAlloc.IsValid(id); }

		template<class T>
		Component<T> AddComponent()
		{
			//static_assert(std::is_base_of<Component<T>, T>::value, "This is not a component type");
			Component<T> c = GetComponent<T>();

			if (c)
			{
				return c;
			}

			return Component<T>::Create(*this);
		}

		template<class T>
		Component<T> GetComponent()
		{
			//static_assert(std::is_base_of<Component<T>, T>::value, "This is not a component type");
			return Component<T>(*this);
		}

		int32_t Destroy();

		static Entity Create();

	private:
		static HandleAllocator<Entity, MAX_ENTITIES> entityAlloc;
	};
}