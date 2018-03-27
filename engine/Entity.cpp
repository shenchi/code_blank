#include "Entity.h"

#include <cassert>

namespace tofu
{
	HandleAllocator<Entity, kMaxEntities> Entity::entityAlloc;

	bool Entity::activeFlags[kMaxEntities] = {};

	int32_t Entity::Destroy()
	{
		assert(!*this);

		entityAlloc.Free(*this);

		// TODO remove all components;
		assert(false && "not implemented yet");

		return kOK;
	}

	Entity Entity::Create()
	{
		Entity e = Entity(entityAlloc.Allocate());
		e.SetActive(true);
		return e;
	}

}