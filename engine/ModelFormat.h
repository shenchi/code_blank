#pragma once

#include <cstdint>
#include "TofuMath.h"
#include <unordered_map>
#include <assert.h>

namespace tofu
{
	namespace model
	{
		typedef std::unordered_map<std::string, uint16_t> AnimationTable;

		constexpr uint32_t kModelFileMagic = 0x004C444D;
		constexpr uint32_t kModelFileVersion = 0x00000001;

		//constexpr uint32_t kModelFileMaxTexcoordChannels = 4;
		constexpr uint32_t kModelFileMaxTexcoordChannels = 1;

		constexpr uint16_t kModelMaxJointIndex = 0x1fff;

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

			// TODO: change type
			uint32_t			NumMeshes;
			uint16_t			NumBones;
			uint32_t			NumAnimations;
			// TODO: Remove following 4 params
			uint32_t            NumAnimChannels;
			uint32_t            NumTotalTranslationFrames;
			uint32_t            NumTotalRotationFrames;
			uint32_t            NumTotalScaleFrames;
			uint32_t			NumAnimationFrames;

			inline uint32_t CalculateVertexSize() const
			{
				uint32_t vertexSize = sizeof(float) * 6; // position + normal;

				if (HasTangent == 1)
				{
					vertexSize += sizeof(float) * 4;
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
		// [tangent]	: float4	- if HasTangent
		// [bone_id]	: int4		- if HasAnimation
		// [weight]		: float4	- if HasAnimation
		// [texcoord1]	: float2	- depending on NumTexcoordChannels
		// [texcoord2]	: float2
		// ...

		// ... followed by indices data, 
		//     index size is 16 bit for now,
		//     index base is 0 for each mesh

		// ... followed by an array of ModelBone struct

		// ... followed by an array of ModelAnimation

		// ... followed by an array of all uniform frames (ModelAnimFrame)

		struct ModelBone
		{
			// TODO: Better solution for name? 
			char			name[128];
			uint16_t		id;
			uint16_t		parent;
			uint16_t		firstChild;
			uint16_t		nextSibling;
			math::float4x4	transform;
			math::float4x4	offsetMatrix;
		};

		struct ModelAnimation
		{
			// TODO: Better solution for name? 
			char			name[128];
			float			tickCount;
			float			ticksPerSecond;
			size_t			startFrames;
			size_t			numFrames;
		};

		enum ChannelType { kChannelTranslation, kChannelRotation, kChannelScale };

		struct ModelAnimFrame
		{
			uint16_t time = 0;

			// TODO: float compression
			// https://github.com/Maratyszcza/FP16/blob/master/third-party/float16-compressor.h
			// https://github.com/guillaumeblanc/ozz-animation/blob/71f622e1480bf76d3cc0da5fe90900dc247234c3/include/ozz/base/maths/internal/simd_math_sse-inl.h
			//// Payload, which can be a quantized 3d vector or a quantized quaternion
			//// depending on what type of channel this data belongs to
			//uint16_t v[3];

			math::float3 value;

		private:
			uint16_t jointIndex = 0;

		public:

			bool GetSignedBit() const 
			{
				// Most significant two bits of this 16bit index contains channel type info.
				return static_cast<bool>((jointIndex & (0x2000)) >> 13);
			}

			void SetSignedBit(bool s)
			{
				jointIndex |= (static_cast<uint16_t>(s == true ? 1 : 0) << 13);
			}

			ChannelType GetChannelType() const
			{
				// Most significant two bits of this 16bit index contains channel type info.
				return static_cast<ChannelType>((jointIndex & (0xc000)) >> 14);
			}
			
			void SetChannelType(ChannelType type)
			{
				jointIndex |= (static_cast<uint16_t>(type) << 14);
			}
			
			std::uint16_t GetJointIndex() const
			{
				return jointIndex & kModelMaxJointIndex;
			}

			void SetJointIndex(uint16_t index) {
				assert(index <= kModelMaxJointIndex);
				jointIndex = (jointIndex & 0xE000) + index;
			}
		};
	}
}

