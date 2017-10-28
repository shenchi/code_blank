#include "Entity.h"

#include <cassert>

namespace tofu
{
	HandleAllocator<Entity, MAX_ENTITIES> Entity::entityAlloc;

	int32_t Entity::Destroy()
	{
		assert(!*this);

		entityAlloc.Free(*this);

		// TODO remove all components;

		return TF_OK;
	}

	Entity Entity::Create()
	{
		return Entity(entityAlloc.Allocate());
	}

}