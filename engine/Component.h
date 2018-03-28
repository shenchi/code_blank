#pragma once

#include "Common.h"
#include "Entity.h"

#include <utility>
#include <cassert>

namespace tofu
{
	struct ComponentIndex
	{
		uint32_t	idx;
		
		explicit ComponentIndex() : idx(UINT32_MAX) {}
	};

	// Component is actually like a handle or pointer, 
	// the actuall component T is in Component<T>::components[]
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

		//inline void Destroy()
		//{
		//	assert(false && "TODO");
		//	assert(true == *this);
		//	
		//	// swap the last element to this location
		//	// change the pointers
		//	// then decrease numComponents

		//	if (numComponents > 1)
		//	{
		//		uint32_t lastElemIdx = numComponents - 1;
		//		Entity lastElemEntity = back_pointers[lastElemIdx];

		//		components[*compIdx] = std::move(components[lastElemIdx]);

		//		back_pointers[lastElemIdx] = Entity();
		//		back_pointers[*compIdx] = lastElemEntity;

		//		pointers[lastElemEntity.id].idx = *compIdx;
		//		*compIdx = UINT32_MAX;
		//	}
		//	else
		//	{
		//		components[*compIdx].~T();
		//		back_pointers[*compIdx] = Entity();
		//		*compIdx = UINT32_MAX;
		//	}

		//	numComponents--;
		//}

		inline void SetActive(bool active)
		{
			assert(true == *this);

			uint32_t compLoc = *compIdx;

			// if this component is waiting to be cleaned up
			if (compLoc >= numActiveComponents + numInactiveComponents)
				return;

			if (active && *compIdx >= numActiveComponents)
			{
				// bring this component back to active
				if (compLoc != numActiveComponents)
				{
					Swap(compLoc, numActiveComponents);
				}
				numActiveComponents++;
				numInactiveComponents--;
			}
			else if (!active && *compIdx < numActiveComponents)
			{
				// put this component into inactive
				if (compLoc != numActiveComponents - 1)
				{
					Swap(compLoc, numActiveComponents - 1);
				}
				numActiveComponents--;
				numInactiveComponents++;
			}
		}

		inline void IsActive() const
		{
			return nullptr != compIdx && *compIdx < numActiveComponents;
		}

		inline void MarkDestroy()
		{
			assert(true == *this);

			uint32_t compLoc = *compIdx;

			// if this component is already waiting to be cleaned up
			uint32_t numAliveComponents = numActiveComponents + numInactiveComponents;
			if (compLoc >= numAliveComponents)
				return;

			// swap the component to end of alive area
			if (compLoc != numAliveComponents - 1)
			{
				Swap(compLoc, numAliveComponents - 1);
			}

			if (compLoc < numActiveComponents)
			{
				// it's an active component
				numActiveComponents--;
			}
			else
			{
				numInactiveComponents--;
			}
		}

	public:
		
		static Component<T> Create(Entity e)
		{
			if (pointers[e.id].idx < numComponents)
				return Component<T>(e);

			if (numComponents >= kMaxEntities)
			{
				return Component<T>();
			}

			uint32_t loc = numComponents++;

			back_pointers[loc] = e;
			pointers[e.id].idx = loc;

			// clear this component in case it was used
			// and prevent resources leaking
			components[loc].~T();
			new (&components[loc]) T(e);

			// swap this component to proper area

			// if there are components waiting to be deleted
			uint32_t numAliveComponents = numActiveComponents + numInactiveComponents;
			if (loc - numAliveComponents > 0)
			{
				// swap to the start of 'waiting to be deleted' area
				Swap(numAliveComponents, loc);
				loc = numAliveComponents;
			}

			// if there are inactive components;
			if (numInactiveComponents > 0)
			{
				// swap to the start of inactive area
				Swap(numActiveComponents, loc);
				loc = numActiveComponents;
			}

			// move active area boundary
			numActiveComponents++;

			return Component<T>(e);
		}

		static void TrimDestroyed()
		{
			uint32_t numAliveComponents = numActiveComponents + numInactiveComponents;
			for (uint32_t i = numAliveComponents; i < numComponents; ++i)
			{
				components[i].~T();
				Entity e = back_pointers[i];
				back_pointers[i] = Entity();
				pointers[e.id].idx = UINT32_MAX;
			}
			numComponents = numAliveComponents;
		}

		static void DestroyByEntity(Entity e)
		{
			Component<T> comp(e);
			if (comp) comp.MarkDestroy();
		}

		static void Swap(Component<T> a, Component<T> b)
		{
			if (!a || !b) return;
			Swap(*a.compIdx, *b.compIdx);
		}

		static void Swap(uint32_t loc_a, uint32_t loc_b)
		{
			if (loc_a == loc_b || loc_a > numComponents || loc_b > numComponents) return;

			Entity e_a = back_pointers[loc_a];
			Entity e_b = back_pointers[loc_b];
			std::swap(components[loc_a], components[loc_b]);
			std::swap(back_pointers[loc_a], back_pointers[loc_b]);
			std::swap(pointers[e_a.id], pointers[e_b.id]);
		}

		static int32_t Init()
		{
			pointers = new ComponentIndex[kMaxEntities];
			back_pointers = new Entity[kMaxEntities];
			components = new T[kMaxEntities];
			numComponents = 0;
			numActiveComponents = 0;
			numInactiveComponents = 0;

			return kOK;
		}

		static int32_t Shutdown()
		{
			numComponents = 0;
			numActiveComponents = 0;
			numInactiveComponents = 0;
			delete[] components;
			delete[] back_pointers;
			delete[] pointers;

			return kOK;
		}

		static int32_t Reset()
		{
			numActiveComponents = 0;
			numInactiveComponents = 0;

			return kOK;
		}

		static T* GetAllComponents() { return components; }

		static uint32_t GetNumComponents() { return numComponents; }

		static uint32_t GetNumActiveComponents() { return numActiveComponents; }

		static uint32_t GetNumInactiveComponents() { return numInactiveComponents; }

	protected:

		// mapping from entity id to component index(location)
		static ComponentIndex* pointers;

		// mapping from component index(location) to entity id
		static Entity* back_pointers;

		// array of actual components
		static T* components;
		static uint32_t numComponents;
		static uint32_t numActiveComponents;
		static uint32_t numInactiveComponents;
	};

	template<class T>
	ComponentIndex* Component<T>::pointers = nullptr;

	template<class T>
	Entity* Component<T>::back_pointers = nullptr;

	template<class T>
	T* Component<T>::components = nullptr;

	template<class T>
	uint32_t Component<T>::numComponents = 0;

	template<class T>
	uint32_t Component<T>::numActiveComponents = 0;

	template<class T>
	uint32_t Component<T>::numInactiveComponents = 0;
}
