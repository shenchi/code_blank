#pragma once

#include "Common.h"
#include "Entity.h"

#include <cassert>

namespace tofu
{
	class Entity;

	struct ComponentIndex
	{
		uint32_t	idx;
		
		explicit ComponentIndex() : idx(UINT32_MAX) {}
	};

	template<class T>
	class Component
	{
	public:
		typedef T component_data_t;
	protected:
		Entity		entity;
		uint32_t*	compIdx;
	
	public:
		Component() : entity(), compIdx(nullptr) {}

		Component(Entity e) : entity(e), compIdx(nullptr) 
		{
			if (e) compIdx = &(pointers[e.id].idx);
		}

		operator bool() const
		{
			return nullptr != compIdx && *compIdx < numComponents;
		}

		T* operator -> () const
		{
			assert(true == *this);
			return &components[*compIdx];
		}

		inline void Destroy()
		{
			assert(true == *this);
			
			// swap the last element to this location
			// change the pointers
			// then decrease numComponents

			if (numComponents > 1)
			{
				uint32_t latsElemIdx = numComponents - 1;
				uint32_t lastElemEntity = back_pointers[latsElemIdx];

				components[*compIdx] = std::move(components[latsElemIdx]);

				back_pointers[lastElemIdx] = Entity();
				back_pointers[*compIdx] = lastElemEntity;

				pointers[lastElemEntity.id].idx = *compIdx;
				*compIdx = UINT32_MAX;
			}
			else
			{
				components[*compIdx].~T();
				back_pointers[*compIdx] = Entity();
				*compIdx = UINT32_MAX;
			}

			numComponents--;
		}

	public:
		
		static Component<T> Create(Entity e)
		{
			if (pointers[e.id].idx < numComponents)
				return Component<T>(e);

			uint32_t loc = numComponents++;

			back_pointers[loc] = e;
			pointers[e.id].idx = loc;

			new (&components[loc]) T(e);

			return Component<T>(e);
		}

		static T* GetAllComponents() { return components; }
		static uint32_t GetNumComponents() { return numComponents; }

	protected:
		static ComponentIndex pointers[MAX_ENTITIES];

		static Entity back_pointers[MAX_ENTITIES];
		static T components[MAX_ENTITIES];
		static uint32_t numComponents;
	};

	template<class T>
	ComponentIndex Component<T>::pointers[MAX_ENTITIES];

	template<class T>
	Entity Component<T>::back_pointers[MAX_ENTITIES];

	template<class T>
	T Component<T>::components[MAX_ENTITIES];

	template<class T>
	uint32_t Component<T>::numComponents = 0;
}
