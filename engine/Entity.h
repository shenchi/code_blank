#pragma once

#include "Common.h"
#include "HandleAllocator.h"

namespace tofu
{
	template<class T>
	class Component;

	// Entity just contans an id
	// and we can find its component directly by this id
	class Entity
	{
	public:
		uint32_t	id;

		Entity(uint32_t _id = MAX_ENTITIES) : id(_id) {}

		// if this eneity is valid
		inline operator bool() const { return entityAlloc.IsValid(id); }

		template<class T>
		Component<typename T::component_data_t> AddComponent()
		{
			//static_assert(std::is_base_of<Component<T>, T>::value, "This is not a component type");
			Component<typename T::component_data_t> c = GetComponent<T>();

			if (c)
			{
				return c;
			}

			return Component<typename T::component_data_t>::Create(*this);
		}

		template<class T>
		Component<typename T::component_data_t> GetComponent() const
		{
			//static_assert(std::is_base_of<Component<T>, T>::value, "This is not a component type");
			return Component<typename T::component_data_t>(*this);
		}

		int32_t Destroy();

		// create a new entity
		static Entity Create();

	private:
		// entity id allocator
		static HandleAllocator<Entity, MAX_ENTITIES> entityAlloc;
	};
}