#include "PathingNode.h"


PathingNode::PathingNode() 
{
	Init();
}

PathingNode::PathingNode(int _index, int _gridX, int _gridY)
{
	index = _index;
	gridX = _gridX;
	gridY = _gridY;

	gCost = 0;
	hCost = 0;
}

// Give me default values.
void PathingNode::Init()
{
	index = -1;
	gridX = -1;
	gridY = -1;

	gCost = 0;
	hCost = 0;

	neighbours[0] = -1;
	neighbours[1] = -1;
	neighbours[2] = -1;
	neighbours[3] = -1;
}

//-------------------------------------------------------------------------------------------------
// Setters

void PathingNode::SetGCost(int cost)
{
	gCost = cost;
}

void PathingNode::SetHCost(int cost)
{
	hCost = cost;
}

void PathingNode::SetCameFrom(PathingNode* node)
{
	cameToThisNodeFrom = node;
}

//-------------------------------------------------------------------------------------------------
// Getters

PathingNode* PathingNode::GetCameFrom()
{
	return cameToThisNodeFrom;
}

// Return my F Cost.
int PathingNode::FCost()
{
	return gCost + hCost;
}

int PathingNode::GCost()
{
	return gCost;
}

int PathingNode::HCost()
{
	return hCost;
}

int PathingNode::GetGridX()
{
	return gridX;
}

int PathingNode::GetGridY()
{
	return gridY;
}

int* PathingNode::GetNeighbours()
{
	return neighbours;
}

// Return my position.
tofu::math::float3 PathingNode::GetPosition()
{
	return nodePosition;
}

// Set the index's of my neighbours, -1 if no neighbor.
void PathingNode::SetConnectedNodes(int first, int second, int third, int fourth)
{
	neighbours[0] = first;
	neighbours[1] = second;
	neighbours[2] = third;
	neighbours[3] = fourth;
}