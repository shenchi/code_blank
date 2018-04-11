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

		Entity(uint32_t _id = kMaxEntities) : id(_id) {}

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

		TF_INLINE bool IsActive() const { return activeFlags[id]; }

		TF_INLINE void SetActive(bool active) { activeFlags[id] = active; }

		TF_INLINE void SetTag(uint32_t _tag) { tags[id] = _tag; }

		TF_INLINE uint32_t getTag() const { return tags[id]; }

		int32_t Destroy();

		// create a new entity
		static Entity Create();

	public:

		// initialize entity-component system
		static int32_t Init();

		// shutdown entity-component system
		static int32_t Shutdown();

		static int32_t Reset();

		typedef int32_t (*InitCallback)(void);
		typedef int32_t (*ShutdownCallback)(void);
		typedef int32_t (*ResetCallback)(void);
		typedef void (*TrimCallback)(void);
		typedef void (*DestroyCallback)(Entity e);

		static void RegisterComponent(InitCallback init, ShutdownCallback shutdown, ResetCallback reset, TrimCallback trim, DestroyCallback destroy);

		template<typename T>
		static void RegisterComponent() {
			RegisterComponent(T::Init, T::Shutdown, T::Reset, T::TrimDestroyed, T::DestroyByEntity);
		}

		// clean up components labeled 'destory'
		static void CleanupComponents();

	private:
		// entity id allocator
		static HandleAllocator<Entity, kMaxEntities> entityAlloc;

		static InitCallback initCallbacks[kMaxComponentTypes];
		static ShutdownCallback shutdownCallbacks[kMaxComponentTypes];
		static ResetCallback resetCallbacks[kMaxComponentTypes];
		static TrimCallback trimCallbacks[kMaxComponentTypes];
		static DestroyCallback destroyCallbacks[kMaxComponentTypes];

		static uint32_t numRegisteredComponentTypes;

		static bool* activeFlags;

		static uint32_t* tags;
	};
}