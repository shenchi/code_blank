#pragma once

#include <cstdint>

namespace tofu
{
	namespace model
	{

		constexpr uint32_t MODEL_FILE_MAGIC = 0x004C444D;
		constexpr uint32_t MODEL_FILE_VERSION = 0x00000001;

		constexpr uint32_t MODEL_FILE_MAX_TEXCOORD_CHANNELS = 4;

		struct ModelHeader
		{
			uint32_t			Magic;
			uint32_t			Version;
			union
			{
				uint32_t		Flags;
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

			inline uint32_t CalculateVertexSize() const
			{
				uint32_t vertexSize = sizeof(float) * 6; // position + normal;

				if (HasTangent == 1)
				{
					vertexSize += sizeof(float) * 3;
				}

				if (HasAnimation == 1)
				{
					vertexSize += sizeof(float) * 4 + sizeof(uint32_t) * 4; // bone ids + bone weights
				}

				vertexSize += NumTexcoordChannels * sizeof(float) * 2;

				return vertexSize;
			}

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

		// ... followed by indices data, 
		//     index size is 16 bit for now,
		//     index base is 0 for each mesh

		// ... followed by an array of bone matrix (the 4th row is omitted)

		// ... followed by an array of ModelAnimation

		struct ModelAnimation
		{
			// TODO
		};
	}
}

