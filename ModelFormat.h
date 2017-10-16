#pragma once

#include <cstdint>

namespace tofu
{
	namespace model
	{

		constexpr uint32_t MODEL_FILE_MAGIC = 0x004C444D;

		constexpr uint32_t MODEL_FILE_MAX_TEXCOORD_CHANNELS = 4;

		struct ModelHeader
		{
			uint32_t			Magic;
			union
			{
				uint32_t		VertexFormatFlags;
				struct
				{
					uint32_t	StructOfArray : 1;
					uint32_t	HasAnimation : 1;
					uint32_t	HasIndices : 1;
					uint32_t	HasTangent : 1;
					uint32_t	_reserved : 24;
					uint32_t	NumTexcoordChannels : 4;
				};
			};
			uint32_t			NumMeshes;
			uint32_t			NumBones;
			uint32_t			NumAnimations;
		};

		// ... followed by an array of ModelMesh struct

		struct ModelMesh
		{
			uint32_t			NumVertices;
			uint32_t			NumIndices;
		};

		// ... followed by vertices data

		// order of channels :
		// position		: float3
		// normal		: float3
		// [tangent]	: float3	- if HasTangent
		// [bone_id]	: int4		- if HasAnimation
		// [weight]		: float4	- if HasAnimation
		// [texcoord1]	: float2	- depending on NumTexcoordChannels
		// [texcoord2]	: float2
		// ...

		// ... followed by indices data

		// ... followed by an array of bone matrix (the 4th row is omitted)

		// ... followed by an array of ModelAnimation

		struct ModelAnimation
		{
			// TODO
		};
	}
}

