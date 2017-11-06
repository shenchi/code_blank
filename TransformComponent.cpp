#include "TransformComponent.h"

namespace tofu
{

	void TransformComponentData::UpdateTransfromInHierachy()
	{
		if (parent)
		{
			worldTransform = localTransform * parent->GetWorldTransform();
		}
		else
		{
			worldTransform = localTransform;
		}

		for (auto i = children.begin(); i != children.end(); ++i)
		{
			(*i)->UpdateTransfromInHierachy();
		}
	}

}

