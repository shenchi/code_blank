#pragma once

#include "common.h"
#include <type_traits>
#include "Component.h"

namespace tofu
{
	template<class T>
	class Component;

	class Entity
	{
	private:
		uint32_t	id;
	public:
		inline uint32_t	Id() const { return id; }
	public:
		template<class T>
		Component<T> AddComponent()
		{
			static_assert(std::is_base_of<Component<T>, T>::value, "This is not a component type");
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
			static_assert(std::is_base_of<Component<T>, T>::value, "This is not a component type");
			return Component<T>{*this};
		}

	public:

		Entity(uint32_t _id = MAX_ENTITIES) : id(_id) {}

		static Entity Create();

		static Entity Invalid;

	private:

		static Entity entities[MAX_ENTITIES];
		static uint32_t numEntities;
	};
}