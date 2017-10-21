#include "Entity.h"

namespace tofu
{

	Entity Entity::entities[MAX_ENTITIES] = {};

	uint32_t Entity::numEntities = 0;

	Entity Entity::Invalid = Entity{ MAX_ENTITIES };

	Entity Entity::Create()
	{
		return Entity(numEntities++); // TODO
	}

}