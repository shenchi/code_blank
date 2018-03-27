#include "TransformComponent.h"

namespace tofu
{
	void TransformComponentData::SetParent(TransformComponent parent, bool updateChildren)
	{
		if (this->parent && this->parent != parent)
		{
			auto& children = this->parent->children;

			for (auto iter = children.begin();
				iter != children.end();
				++iter)
			{
				if ((*iter)->entity == entity)
				{
					children.erase(iter);
					break;
				}
			}
		}

		this->parent = parent;
		if (parent)
		{
			parent->children.push_back(
				entity.GetComponent<TransformComponent>()
			);
		}

		if (updateChildren)
			UpdateTransfromInHierachy();
	}

	void TransformComponentData::AddChild(TransformComponent child)
	{
		child->SetParent(entity.GetComponent<TransformComponent>());
	}

	void TransformComponentData::RemoveChild(TransformComponent child)
	{
		child->SetParent(TransformComponent());
	}

	void TransformComponentData::RemoveChild(uint32_t childIdx)
	{
		TransformComponent child = children[childIdx];
		child->SetParent(TransformComponent());
	}

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

