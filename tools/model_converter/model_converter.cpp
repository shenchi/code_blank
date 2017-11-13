#include <cstdio>
#include <cstdint>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <unordered_map>

#pragma comment (lib, "assimp-vc140-mt.lib")

#include "../../ModelFormat.h"
#include "../../TofuMath.h"

using tofu::math::float2;
using tofu::math::float3;
using tofu::math::float4;
using tofu::math::quat;
using tofu::math::float4x4;
using tofu::math::int4;

struct Bone
{
	uint32_t id;
	uint32_t parent;
	uint32_t firstChild;
	uint32_t nextSibling;
	float4x4 transform;
	float4x4 offsetMatrix;
};

typedef std::vector<Bone> BoneTree;
typedef std::unordered_map<std::string, uint32_t> BoneTable;

inline void CopyMatrix(float4x4& dstMatrix, const aiMatrix4x4& srcMatrix)
{
	memcpy(&dstMatrix, &srcMatrix, sizeof(float4x4));
}

inline bool IsIdentity(const float4x4& mat)
{
	const float* m = reinterpret_cast<const float*>(&mat);
	for (uint32_t i = 0; i < 16; i++)
	{
		if ((i % 5 == 0 && m[i] != 1.0f) || (i % 5 != 0 && m[i] != 0.0f))
			return false;
	}
	return true;
}

inline bool IsEqual(const float4x4& a, const aiMatrix4x4& b)
{
	const float* ma = reinterpret_cast<const float*>(&a);
	const float* mb = reinterpret_cast<const float*>(&b);
	for (uint32_t i = 0; i < 16; i++)
	{
		if (std::fabsf(ma[i] - mb[i]) > FLT_EPSILON)
			return false;
	}
	return true;
}

uint32_t loadBoneHierarchy(aiNode* node, BoneTree& bones, BoneTable& table, uint32_t parentId = UINT32_MAX, uint32_t lastSibling = UINT32_MAX)
{
	uint32_t boneId = static_cast<uint32_t>(bones.size());
	bones.push_back(Bone());
	Bone& bone = bones[boneId];
	bone.id = boneId;
	bone.parent = parentId;
	bone.nextSibling = UINT32_MAX;

	if (lastSibling != UINT32_MAX)
	{
		bones[lastSibling].nextSibling = boneId;
	}

	if (node->mName.length > 0)
	{
		table.insert(std::pair<std::string, uint32_t>(node->mName.C_Str(), boneId));
	}

	CopyMatrix(bone.transform, node->mTransformation);
	bone.offsetMatrix = tofu::math::matrix::identity();

	uint32_t firstChild = UINT32_MAX;
	uint32_t lastChild = UINT32_MAX;
	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		uint32_t id = loadBoneHierarchy(node->mChildren[i], bones, table, boneId, lastChild);
		if (firstChild == UINT32_MAX)
		{
			firstChild = id;
		}
		lastChild = id;
	}
	bones[boneId].firstChild = firstChild;

	return boneId;
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("model_converter input_file output_file");
		return 0;
	}

	unsigned int flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace;

	bool bConvertToLeftHanded = false;
	bool bStructureOfArray = false;

	if (bConvertToLeftHanded)
	{
		flags |= aiProcess_ConvertToLeftHanded;
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(std::string(argv[1]), flags);

	if (nullptr == scene)
	{
		printf("failed to read input file.\n%s\n", importer.GetErrorString());
		return __LINE__;
	}

	FILE* file = fopen(argv[2], "wb");
	if (nullptr == file)
	{
		printf("failed to create output file.\n");
		return __LINE__;
	}

	tofu::model::ModelHeader header = {};
	header.Magic = tofu::model::MODEL_FILE_MAGIC;
	header.Version = tofu::model::MODEL_FILE_VERSION;
	header.StructOfArray = bStructureOfArray ? 1 : 0;
	header.HasIndices = 1;
	header.HasTangent = 1;
	header.NumTexcoordChannels = 0;

	header.NumMeshes = scene->mNumMeshes;
	header.NumBones = 0;
	header.NumAnimations = 0;

	BoneTree bones;
	BoneTable boneTable;

	if (scene->mNumAnimations > 0 && scene->mRootNode->mNumChildren > 0)
	{
		// load bone hierarchy
		loadBoneHierarchy(scene->mRootNode, bones, boneTable);
		
		header.NumBones = static_cast<uint32_t>(bones.size());
		header.NumAnimations = scene->mNumAnimations;
		header.HasAnimation = 1;
	}

	uint32_t vertexSize = 0;
	uint32_t numVertices = 0, numIndices = 0;

	// get number of texcoord channels
	for (uint32_t i = 0; i < header.NumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		uint32_t numUVChns = mesh->GetNumUVChannels();
		if (numUVChns > header.NumTexcoordChannels)
		{
			header.NumTexcoordChannels = numUVChns;
		}
	}

	if (header.NumTexcoordChannels > tofu::model::MODEL_FILE_MAX_TEXCOORD_CHANNELS)
	{
		header.NumTexcoordChannels = tofu::model::MODEL_FILE_MAX_TEXCOORD_CHANNELS;
	}

	// writing header
	if (1 != fwrite(&header, sizeof(header), 1, file))
	{
		printf("failed to write header data.\n");
		return __LINE__;
	}

	// writing mesh data
	for (uint32_t i = 0; i < header.NumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		tofu::model::ModelMesh meshInfo = {};
		meshInfo.NumVertices = mesh->mNumVertices;
		meshInfo.NumIndices = mesh->mNumFaces * 3;

		numVertices += meshInfo.NumVertices;
		numIndices += meshInfo.NumIndices;

		if (1 != fwrite(&meshInfo, sizeof(meshInfo), 1, file))
		{
			printf("failed to write mesh data.\n");
			return __LINE__;
		}
	}

	// calculate vertex size;
	{
		vertexSize = header.CalculateVertexSize();
	}

	// allocate buffer for vertices and indices
	uint32_t verticesDataSize = vertexSize * numVertices;
	uint32_t indicesDataSize = sizeof(uint16_t) * numIndices;
	uint32_t bufferSize = verticesDataSize + indicesDataSize;
	void* buffer = malloc(bufferSize);
	if (nullptr == buffer)
	{
		printf("failed to allocate buffer.\n");
		return __LINE__;
	}

	// populate vertices data
	if (bStructureOfArray)
	{
		printf("not implemented yet.\n");
		return __LINE__;
	}
	else
	{
		uint8_t* ptr = reinterpret_cast<uint8_t*>(buffer);
		for (uint32_t i = 0; i < header.NumMeshes; ++i)
		{
			const aiMesh* mesh = scene->mMeshes[i];
			uint8_t* meshBaseAddr = ptr;

			for (uint32_t j = 0; j < mesh->mNumVertices; ++j)
			{
				aiVector3D& pos = mesh->mVertices[j];
				aiVector3D& nor = mesh->mNormals[j];

				*reinterpret_cast<float3*>(ptr) = float3{ pos.x, pos.y, pos.z };
				ptr += sizeof(float3);

				*reinterpret_cast<float3*>(ptr) = float3{ nor.x, nor.y, nor.z };
				ptr += sizeof(float3);

				if (header.HasTangent == 1)
				{
					aiVector3D& tan = mesh->mTangents[j];

					*reinterpret_cast<float3*>(ptr) = float3{ tan.x, tan.y, tan.z };
					ptr += sizeof(float3);
				}

				if (header.HasAnimation)
				{
					// reserve space for bone Ids and weights
					*reinterpret_cast<int4*>(ptr) = int4{ -1, -1, -1, -1 };
					ptr += sizeof(int4);
					*reinterpret_cast<float4*>(ptr) = float4();
					ptr += sizeof(float4);
				}

				for (uint32_t k = 0; k < header.NumTexcoordChannels; ++k)
				{
					if (k < mesh->GetNumUVChannels())
					{
						aiVector3D& uv = mesh->mTextureCoords[k][j];
						*reinterpret_cast<float2*>(ptr) = float2{ uv.x, uv.y };
						ptr += sizeof(float2);
					}
					else
					{
						*reinterpret_cast<float2*>(ptr) = float2{ 0.0f, 0.0f };
						ptr += sizeof(float2);
					}
				}
			}

			if (!header.HasAnimation)
			{
				continue;
			}

			for (uint32_t j = 0; j < mesh->mNumBones; ++j)
			{
				aiBone* bone = mesh->mBones[j];
				std::string boneName(bone->mName.C_Str());

				auto iter = boneTable.find(boneName);
				if (iter == boneTable.end())
				{
					printf("cannot find bone '%s' in hierarchy.\n", boneName.c_str());
					return __LINE__;
				}

				uint32_t boneId = iter->second;

				if (IsIdentity(bones[boneId].offsetMatrix))
				{
					CopyMatrix(bones[boneId].offsetMatrix, bone->mOffsetMatrix);
				}
				else if (!IsEqual(bones[boneId].offsetMatrix, bone->mOffsetMatrix))
				{
					printf("different offset matrix for the same bone %d.", boneId);
					return __LINE__;
				}

				for (uint32_t k = 0; k < bone->mNumWeights; k++)
				{
					uint32_t vertexId = bone->mWeights[k].mVertexId;
					float weight = bone->mWeights[k].mWeight;

					int4* boneIds = reinterpret_cast<int4*>(meshBaseAddr
						+ vertexSize * vertexId 
						+ sizeof(float3) * (header.HasTangent ? 3 : 2));

					float4* weights = reinterpret_cast<float4*>(boneIds + 1);

					if (boneIds->x == -1)
					{
						boneIds->x = boneId;
						weights->x = weight;
					}
					else if (boneIds->y == -1)
					{
						boneIds->y = boneId;
						weights->y = weight;
					}
					else if (boneIds->z == -1)
					{
						boneIds->z = boneId;
						weights->z = weight;
					}
					else if (boneIds->w == -1)
					{
						boneIds->w = boneId;
						weights->w = weight;
					}
					else
					{
						printf("mesh %d vertex %d has more than 4 bones bound.", i, vertexId);
						return __LINE__;
					}
				}

			}
		}
	}

	// populate indices data
	{
		uint16_t* indices = reinterpret_cast<uint16_t*>(
			reinterpret_cast<uint8_t*>(buffer) + verticesDataSize
			);

		for (uint32_t i = 0; i < header.NumMeshes; ++i)
		{
			const aiMesh* mesh = scene->mMeshes[i];

			for (uint32_t j = 0; j < mesh->mNumFaces; ++j)
			{
				uint32_t a = mesh->mFaces[j].mIndices[0];
				uint32_t b = mesh->mFaces[j].mIndices[1];
				uint32_t c = mesh->mFaces[j].mIndices[2];

				if (a > UINT16_MAX || b > UINT16_MAX || c > UINT16_MAX)
				{
					printf("index larger overflow 16 bits.\n");
					return __LINE__;
				}

				*indices = a;
				indices++;
				*indices = b;
				indices++;
				*indices = c;
				indices++;
			}
		}
	}

	// write buffer to file
	if (1 != fwrite(buffer, bufferSize, 1, file))
	{
		printf("failed to write buffer to file.\n");
		return __LINE__;
	}
	free(buffer);

	// TODO bone data
	 
	// TODO animation data

	importer.FreeScene();
	fclose(file);

	return 0;
}