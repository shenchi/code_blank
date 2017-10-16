#include <cstdio>
#include <cstdint>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#pragma comment (lib, "assimp-vc140-mt.lib")

#include "../../ModelFormat.h"
#include "../../TofuMath.h"

using tofu::math::float2;
using tofu::math::float3;
using tofu::math::float4;
using tofu::math::uint4;

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
		return -1;
	}

	FILE* file = fopen(argv[2], "wb");
	if (nullptr == file)
	{
		printf("failed to create output file.\n");
		return -1;
	}

	tofu::model::ModelHeader header = {};
	header.Magic = tofu::model::MODEL_FILE_MAGIC;
	header.Version = tofu::model::MODEL_FILE_VERSION;
	header.StructOfArray = bStructureOfArray ? 1 : 0;
	header.HasIndices = 1;
	header.HasTangent = 1;
	header.NumTexcoordChannels = 0;

	header.NumMeshes = scene->mNumMeshes;
	header.NumBones = 0; // TODO
	header.NumAnimations = 0; // TODO

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
		return -1;
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
			return -1;
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
		return -3;
	}

	// populate vertices data
	if (bStructureOfArray)
	{
		printf("not implemented yet.\n");
		return -2;
	}
	else
	{
		uint8_t* ptr = reinterpret_cast<uint8_t*>(buffer);
		for (uint32_t i = 0; i < header.NumMeshes; ++i)
		{
			const aiMesh* mesh = scene->mMeshes[i];
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

				// TODO bone ids and weights

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
		}
	}

	// populate indices data
	{
		uint16_t* indices = reinterpret_cast<uint16_t*>(
			reinterpret_cast<uint8_t*>(buffer) + vertexSize
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
					return -4;
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
		return -1;
	}
	free(buffer);

	// TODO bone data

	// TODO animation data

	importer.FreeScene();
	fclose(file);

	return 0;
}