#pragma once

#include "Common.h"

namespace tofu
{
	struct Mesh
	{
		uint32_t	vb;
		uint32_t	ib;
		uint32_t	startVertex;
		uint32_t	numVertices;
		uint32_t	startIndex;
		uint32_t	numIndices;
	};

}