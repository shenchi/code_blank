#include "NavigationSingleton.h"
#include <unordered_set>
#include <assert.h>

NavigationSingleton* NavigationSingleton::instance = 0;

NavigationSingleton::NavigationSingleton()
{}

// Return instance of class
NavigationSingleton* NavigationSingleton::Instance()
{
	if (instance == 0)
	{
		instance = new NavigationSingleton();
	}

	return instance;
}

// Setup class
void NavigationSingleton::Init()
{}

// Returns the current node that you are standing over.
PathingNode* NavigationSingleton::GetCurrentNode(tofu::math::float3 currentPosition)
{

	PathingNode* returnNode = nullptr;
	float temporaryDistance = 1000;

	// This is not ideal as we call this quite frequently and we do not want to perform this same function over and over again unless you actually optimise this.
	for (int i = 0; i < nodes->size(); i++)
	{
		if (temporaryDistance > tofu::math::distance(currentPosition, nodes->at(i).GetPosition() ))
		{
			temporaryDistance = tofu::math::distance(currentPosition, nodes->at(i).GetPosition());
			returnNode = &nodes->at(i);
		}
	}

	assert(nullptr != returnNode);
	return returnNode;
}

// Find the path between two one to another node. nodeA is the Start Node. nodeB is the endNode.
std::vector<PathingNode*> NavigationSingleton::GetPath(PathingNode* nodeA, PathingNode* nodeB)
{
	// Assume that the distance between two adjacent nodes is 1. We will be using A* method for Pathfinding.
	/*
	std::vector<PathingNode*> openSet;
	//HashSet<PathingNode> closedSet = new HashSet<PathingNode>();
	std::unordered_set<int> closedSet = {};
	int nodeIndex = 0;

	openSet.push_back(nodeA);
	nodeA->SetGCost(0);

	while (openSet.size() > 0)
	{

		PathingNode* currentNode = openSet.at(0);

		for (uint32_t i = 1; i < openSet.size(); i++)
		{
			if (openSet.at(i)->FCost() < currentNode->FCost() || ((openSet.at(i)->FCost() == currentNode->FCost()) && openSet.at(i)->HCost() < currentNode->HCost()))
			{
				currentNode = openSet.at(i);
				nodeIndex = i;
			}
		}

		if (nodeB == currentNode)
		{
			// TODO: What is pushed onto return Path.
			std::vector<PathingNode*> returnPath = RetracePath(nodeA, nodeB);
			return returnPath;
		}

		openSet.erase(openSet.begin() + (nodeIndex - 1));
		closedSet.emplace(currentNode);
		//openSet.Remove(currentNode);
		//closedSet.Add(currentNode);
		
		for (uint32_t i = 0; i < 4; i++)
		{
			std::unordered_set<int>::const_iterator neighbour = closedSet.find(currentNode->GetIndex());

			if ( !(neighbour == closedSet.end()) )
			{
				assert(nullptr != currentNode);
				assert(nullptr != nodeB);
				assert(nullptr != neighbour._Ptr);

				PathingNode node = neighbour;

				int newMovementCostToNeighbour = currentNode->GCost() + GetDistance(currentNode, &node);
				if (newMovementCostToNeighbour < node.GCost() ) 
				{
					node.SetGCost(newMovementCostToNeighbour);
					node.SetHCost(GetDistance(&node, nodeB));
					node.SetCameFrom(currentNode);

					bool contains = false;
					for (uint32_t i = 0; i < openSet.size(); i++)
					{
						if (openSet.at(i) == &node)
						{
							contains = true;
						}
					}
					if (!contains)
					{
						openSet.push_back(&node);
					}

				}
				else
				{
					for (uint32_t i = 0; i < openSet.size(); i++)
					{
						if (openSet.at(i) == &node)
						{
							node.SetGCost(newMovementCostToNeighbour);
							node.SetHCost(GetDistance(&node, nodeB));
							node.SetCameFrom(currentNode);

							bool contains = false;
							for (uint32_t i = 0; i < openSet.size(); i++)
							{
								if (openSet.at(i) == &node)
								{
									contains = true;
								}
							}
							if (!contains)
							{
								openSet.push_back(&node);
							}
						}
					}
				}
			}
			else
			{
				// Not found.
			}
		}
	}*/

	// Well, if you reach this. There is no path..
	//assert(false);

	// Return an empty path.
	std::vector<PathingNode*> returnPath{};
	return returnPath;

}


std::vector<PathingNode*> NavigationSingleton::RetracePath(PathingNode* nodeA, PathingNode* nodeB)
{
	std::vector<PathingNode*> returnPath;
	PathingNode* currentNode = nodeB;

	while (nodeA != currentNode)
	{
		returnPath.push_back(currentNode);
		assert(nullptr != currentNode->GetCameFrom());
		//Debug.Assert(null != currentNode.cameToThisNodeFrom, "This path is broken / incomplete.");
		currentNode = currentNode->GetCameFrom();
	}

	returnPath.reserve(returnPath.size());
	return returnPath;
}

int NavigationSingleton::GetDistance(PathingNode* nodeA, PathingNode* nodeB)
{

	assert(nullptr != nodeA);
	assert(nullptr != nodeB);

	int distX = tofu::math::abs(nodeA->GetGridX() - nodeB->GetGridX() );
	int distY = tofu::math::abs(nodeA->GetGridY() - nodeB->GetGridY() );

	return (distX > distY) ? (14 * distY + 10 * (distX - distY)) : (14 * distX + 10 * (distY - distX));
}

void NavigationSingleton::SetNodes(std::vector<PathingNode>* _nodes)
{
	nodes = _nodes;
}

bool NavigationSingleton::GetCanCalculate()
{
	return canCalculate;
}