#include "Entity.h"

#include "TransformComponent.h"

#include <cassert>

namespace tofu
{
	HandleAllocator<Entity, kMaxEntities> Entity::entityAlloc;

	Entity::InitCallback Entity::initCallbacks[kMaxComponentTypes];
	Entity::ShutdownCallback Entity::shutdownCallbacks[kMaxComponentTypes];
	Entity::ResetCallback Entity::resetCallbacks[kMaxComponentTypes];
	Entity::TrimCallback Entity::trimCallbacks[kMaxComponentTypes];
	Entity::DestroyCallback Entity::destroyCallbacks[kMaxComponentTypes];

	uint32_t Entity::numRegisteredComponentTypes = 0;

	bool* Entity::activeFlags = nullptr;
	uint32_t* Entity::tags = nullptr;

	int32_t Entity::Destroy()
	{
		if (!*this)	return kErrUnknown;

		TransformComponent t = GetComponent<TransformComponent>();
		if (t)
		{
			t->SetParent(TransformComponent(), false);
			uint32_t numChildren = t->GetNumChildren();
			for (uint32_t i = 0; i < numChildren; i++)
			{
				Entity child = t->GetChild(i)->GetEntity();
				if (child) child.Destroy();
			}
		}

		for (uint32_t i = 0; i < numRegisteredComponentTypes; i++)
		{
			destroyCallbacks[i](*this);
		}

		entityAlloc.Free(*this);

		return kOK;
	}

	Entity Entity::Create()
	{
		Entity e = Entity(entityAlloc.Allocate());
		e.SetActive(true);
		return e;
	}

	int32_t Entity::Init()
	{
		activeFlags = new bool[kMaxEntities] { false };
		tags = new uint32_t[kMaxEntities]{ 0 };

		// initialize component groups
		for (uint32_t i = 0; i < numRegisteredComponentTypes; i++)
		{
			CHECKED(initCallbacks[i]());
		}

		return kOK;
	}

	int32_t Entity::Shutdown()
	{
		// shutdown all component groups
		for (uint32_t i = 0; i < numRegisteredComponentTypes; i++)
		{
			CHECKED(shutdownCallbacks[i]());
		}

		delete[] activeFlags;
		delete[] tags;

		return kOK;
	}

	int32_t Entity::Reset()
	{
		std::fill(activeFlags, activeFlags + kMaxEntities, false);
		std::fill(tags, tags + kMaxEntities, 0);

		for (uint32_t i = 0; i < numRegisteredComponentTypes; i++)
		{
			CHECKED(resetCallbacks[i]());
		}

		return int32_t();
	}

	void Entity::RegisterComponent(InitCallback init, ShutdownCallback shutdown, ResetCallback reset, TrimCallback trim, DestroyCallback destroy)
	{
		uint32_t i = numRegisteredComponentTypes++;

		initCallbacks[i] = init;
		shutdownCallbacks[i] = shutdown;
		resetCallbacks[i] = reset;
		trimCallbacks[i] = trim;
		destroyCallbacks[i] = destroy;
	}

	void Entity::CleanupComponents()
	{
		for (uint32_t i = 0; i < numRegisteredComponentTypes; i++)
		{
			trimCallbacks[i]();
		}
	}
}