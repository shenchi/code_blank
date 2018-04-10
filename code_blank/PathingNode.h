// Darren Farr
// AI Conversion, original by Sravan Kuraturi in C#
#pragma once
#include<TofuMath.h>
#include <vector>


// Path nodes for AI system.
class PathingNode
{
public:
	PathingNode();
	PathingNode(int, int, int);

	void Init();
	void SetConnectedNodes(int, int, int, int);
	void SetGCost(int);
	void SetHCost(int);
	void SetCameFrom(PathingNode*);
	
	PathingNode* GetCameFrom();
	int FCost();
	int GCost();
	int HCost();
	int GetGridX();
	int GetGridY();
	int* GetNeighbours();
	tofu::math::float3 GetPosition();

private:

	int index;

	tofu::math::float3 nodePosition;

	int gridX;	// Used to calculate the distance between two different nodes.
	int gridY;

	// Neighbours.
	int neighbours[4];

	// A*
	// Pathing Variables
	int gCost;
	int hCost;

	// The node from where you came to this node.
	PathingNode* cameToThisNodeFrom;
};