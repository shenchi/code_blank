// Darren Farr
// AI Conversion, original by Sravan Kuraturi in C#
#pragma once
#include <TofuMath.h>
#include <vector>
#include "PathingNode.h"



class NavigationSingleton
{
public:
	static NavigationSingleton* Instance();


	void Init();
	void SetNodes(std::vector<PathingNode>* _nodes);

	PathingNode* GetCurrentNode(tofu::math::float3);
	std::vector<PathingNode*> GetPath(PathingNode*, PathingNode*);
	std::vector<PathingNode*> RetracePath(PathingNode*, PathingNode*);
	int GetDistance(PathingNode*, PathingNode*);
	bool GetCanCalculate();

private:
	static NavigationSingleton* instance;
	NavigationSingleton();



	// This is a flag to determine if we can calculate the path or not. If one enemy is calculating the path. 
	//Another can't because the fCost and hCost is dependant on the start position. Also, the parent or the node where it came from.
	bool canCalculate = true;

	// This is the pointer to the existing nodes. This would be filled upfront on the engine from the exporter.
	std::vector<PathingNode>* nodes;
};