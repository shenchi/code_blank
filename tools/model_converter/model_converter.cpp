#include <cstdio>
#include <cstdint>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <math.h> 

#include <rapidjson/document.h>

#pragma comment (lib, "assimp-vc140-mt.lib")

#define TOFU_USE_GLM

#include "../../engine/ModelFormat.h"
#include "../../engine/TofuMath.h"
#include "../../engine/Compression.h"

//using tofu::math::float2;
//using tofu::math::float3;
//using tofu::math::float4;
//using tofu::math::quat;
//using tofu::math::float4x4;
//using tofu::math::int4;
//using tofu::math::uint4;

typedef tofu::model::ModelMesh Mesh;
typedef tofu::model::ModelBone Bone;
typedef tofu::model::ModelAnimation Animation;

typedef std::vector<Bone> BoneTree;
typedef std::unordered_map<std::string, uint16_t> BoneTable;

using namespace tofu::math;
using namespace tofu::model;
using namespace tofu::compression;

struct HumanBone
{
	enum HumanBoneId
	{
		None = -1,
		Root,
		Hips,
		Spine,
		Spine1,
		Spine2,
		Neck,
		Head,
		HeadTop,
		LeftShoulder,
		LeftArm,
		LeftForeArm,
		LeftHand,
		LeftHandThumb1,
		LeftHandThumb2,
		LeftHandThumb3,
		LeftHandThumb4,
		LeftHandIndex1,
		LeftHandIndex2,
		LeftHandIndex3,
		LeftHandIndex4,
		LeftHandMiddle1,
		LeftHandMiddle2,
		LeftHandMiddle3,
		LeftHandMiddle4,
		LeftHandRing1,
		LeftHandRing2,
		LeftHandRing3,
		LeftHandRing4,
		LeftHandPinky1,
		LeftHandPinky2,
		LeftHandPinky3,
		LeftHandPinky4,
		RightShoulder,
		RightArm,
		RightForeArm,
		RightHand,
		RightHandThumb1,
		RightHandThumb2,
		RightHandThumb3,
		RightHandThumb4,
		RightHandIndex1,
		RightHandIndex2,
		RightHandIndex3,
		RightHandIndex4,
		RightHandMiddle1,
		RightHandMiddle2,
		RightHandMiddle3,
		RightHandMiddle4,
		RightHandRing1,
		RightHandRing2,
		RightHandRing3,
		RightHandRing4,
		RightHandPinky1,
		RightHandPinky2,
		RightHandPinky3,
		RightHandPinky4,
		LeftUpLeg,
		LeftLeg,
		LeftFoot,
		LeftToeBase,
		LeftToeBaseEnd,
		RightUpLeg,
		RightLeg,
		RightFoot,
		RightToeBase,
		RightToeBaseEnd,
		NumHumanBones
	};

	static uint32_t				parent(uint32_t id);
	static uint32_t				target(uint32_t id);
	static const float3&		direction(uint32_t id);
	static const char*			name(uint32_t id);
};

const char* HumanBoneNames[HumanBone::NumHumanBones] =
{
	"Root",
	"Hips",
	"Spine",
	"Spine1",
	"Spine2",
	"Neck",
	"Head",
	"HeadTop",
	"LeftShoulder",
	"LeftArm",
	"LeftForeArm",
	"LeftHand",
	"LeftHandThumb1",
	"LeftHandThumb2",
	"LeftHandThumb3",
	"LeftHandThumb4",
	"LeftHandIndex1",
	"LeftHandIndex2",
	"LeftHandIndex3",
	"LeftHandIndex4",
	"LeftHandMiddle1",
	"LeftHandMiddle2",
	"LeftHandMiddle3",
	"LeftHandMiddle4",
	"LeftHandRing1",
	"LeftHandRing2",
	"LeftHandRing3",
	"LeftHandRing4",
	"LeftHandPinky1",
	"LeftHandPinky2",
	"LeftHandPinky3",
	"LeftHandPinky4",
	"RightShoulder",
	"RightArm",
	"RightForeArm",
	"RightHand",
	"RightHandThumb1",
	"RightHandThumb2",
	"RightHandThumb3",
	"RightHandThumb4",
	"RightHandIndex1",
	"RightHandIndex2",
	"RightHandIndex3",
	"RightHandIndex4",
	"RightHandMiddle1",
	"RightHandMiddle2",
	"RightHandMiddle3",
	"RightHandMiddle4",
	"RightHandRing1",
	"RightHandRing2",
	"RightHandRing3",
	"RightHandRing4",
	"RightHandPinky1",
	"RightHandPinky2",
	"RightHandPinky3",
	"RightHandPinky4",
	"LeftUpLeg",
	"LeftLeg",
	"LeftFoot",
	"LeftToeBase",
	"LeftToeBaseEnd",
	"RightUpLeg",
	"RightLeg",
	"RightFoot",
	"RightToeBase",
	"RightToeBaseEnd",
};

HumanBone::HumanBoneId HumanBoneDirectionTargets[HumanBone::NumHumanBones] =
{
	HumanBone::Hips, //Root,
	HumanBone::Spine, //Hips,
	HumanBone::Spine1, //Spine,
	HumanBone::Spine2, //Spine1,
	HumanBone::Neck, //Spine2,
	HumanBone::Head, //Neck,
	HumanBone::HeadTop, //Head,
	HumanBone::None, //HeadTop,
	HumanBone::LeftArm, //LeftShoulder,
	HumanBone::LeftForeArm, //LeftArm,
	HumanBone::LeftHand, //LeftForeArm,
	HumanBone::LeftHandMiddle1, //LeftHand,
	HumanBone::LeftHandThumb2, //LeftHandThumb1,
	HumanBone::LeftHandThumb3, //LeftHandThumb2,
	HumanBone::LeftHandThumb4, //LeftHandThumb3,
	HumanBone::None, //LeftHandThumb4,
	HumanBone::LeftHandIndex2, //LeftHandIndex1,
	HumanBone::LeftHandIndex3, //LeftHandIndex2,
	HumanBone::LeftHandIndex4, //LeftHandIndex3,
	HumanBone::None, //LeftHandIndex4,
	HumanBone::LeftHandMiddle2, //LeftHandMiddle1,
	HumanBone::LeftHandMiddle3, //LeftHandMiddle2,
	HumanBone::LeftHandMiddle4, //LeftHandMiddle3,
	HumanBone::None, //LeftHandMiddle4,
	HumanBone::LeftHandRing2, //LeftHandRing1,
	HumanBone::LeftHandRing3, //LeftHandRing2,
	HumanBone::LeftHandRing4, //LeftHandRing3,
	HumanBone::None, //LeftHandRing4,
	HumanBone::LeftHandPinky2, //LeftHandPinky1,
	HumanBone::LeftHandPinky3, //LeftHandPinky2,
	HumanBone::LeftHandPinky4, //LeftHandPinky3,
	HumanBone::None, //LeftHandPinky4,
	HumanBone::RightArm, //RightShoulder,
	HumanBone::RightForeArm, //RightArm,
	HumanBone::RightHand, //RightForeArm,
	HumanBone::RightHandMiddle1, //RightHand,
	HumanBone::RightHandThumb2, //RightHandThumb1,
	HumanBone::RightHandThumb3, //RightHandThumb2,
	HumanBone::RightHandThumb4, //RightHandThumb3,
	HumanBone::None, //RightHandThumb4,
	HumanBone::RightHandIndex2, //RightHandIndex1,
	HumanBone::RightHandIndex3, //RightHandIndex2,
	HumanBone::RightHandIndex4, //RightHandIndex3,
	HumanBone::None, //RightHandIndex4,
	HumanBone::RightHandMiddle2, //RightHandMiddle1,
	HumanBone::RightHandMiddle3, //RightHandMiddle2,
	HumanBone::RightHandMiddle4, //RightHandMiddle3,
	HumanBone::None, //RightHandMiddle4,
	HumanBone::RightHandRing2, //RightHandRing1,
	HumanBone::RightHandRing3, //RightHandRing2,
	HumanBone::RightHandRing4, //RightHandRing3,
	HumanBone::None, //RightHandRing4,
	HumanBone::RightHandPinky2, //RightHandPinky1,
	HumanBone::RightHandPinky3, //RightHandPinky2,
	HumanBone::RightHandPinky4, //RightHandPinky3,
	HumanBone::None, //RightHandPinky4,
	HumanBone::LeftLeg, //LeftUpLeg,
	HumanBone::LeftFoot, //LeftLeg,
	HumanBone::LeftToeBase, //LeftFoot,
	HumanBone::LeftToeBaseEnd, //LeftToeBase,
	HumanBone::None, //LeftToeBaseEnd,
	HumanBone::RightLeg, //RightUpLeg,
	HumanBone::RightFoot, //RightLeg,
	HumanBone::RightToeBase, //RightFoot,
	HumanBone::RightToeBaseEnd, //RightToeBase,
	HumanBone::None, //RightToeBaseEnd,
};

float3 HumanBoneDefaultDirections[HumanBone::NumHumanBones] =
{
	{}, //Root, (doesn't matter)
	{ 0, 1, 0 }, //Hips,
	{ 0, 1, 0 }, //Spine,
	{ 0, 1, 0 }, //Spine1,
	{ 0, 1, 0 }, //Spine2,
	{ 0, 1, 0 }, //Neck,
	{ 0, 1, 0 }, //Head,
	{}, //HeadTop, (doesn't matter)
	{ 1, 0, 0 }, //LeftShoulder,
	{ 1, 0, 0 }, //LeftArm,
	{ 1, 0, 0 }, //LeftForeArm,
	{ 0.275f, 0.016f, -0.041f }, //LeftHand,
	{ 0.079f, -0.017f, -0.022f }, //LeftHandThumb1,
	{ 1, 0, 0 }, //LeftHandThumb2,
	{ 1, 0, 0 }, //LeftHandThumb3,
	{}, //LeftHandThumb4, (doesn't matter)
	{ 0.132f, 0, -0.005f }, //LeftHandIndex1,
	{ 0.083f, 0, -0.003f }, //LeftHandIndex2,
	{ 0.061f, 0, -0.002f }, //LeftHandIndex3,
	{}, //LeftHandIndex4, (doesn't matter)
	{ 1, 0, 0 }, //LeftHandMiddle1,
	{ 1, 0, 0 }, //LeftHandMiddle2,
	{ 1, 0, 0 }, //LeftHandMiddle3,
	{}, //LeftHandMiddle4, (doesn't matter)
	{ 1, 0, 0 }, //LeftHandRing1,
	{ 1, 0, 0 }, //LeftHandRing2,
	{ 1, 0, 0 }, //LeftHandRing3,
	{}, //LeftHandRing4, (doesn't matter)
	{ 1, 0, 0 }, //LeftHandPinky1,
	{ 1, 0, 0 }, //LeftHandPinky2,
	{ 1, 0, 0 }, //LeftHandPinky3,
	{}, //LeftHandPinky4, (doesn't matter)
	{ -1, 0, 0 }, //RightShoulder,
	{ -1, 0, 0 }, //RightArm,
	{ -1, 0, 0 }, //RightForeArm,
	{ -0.275f, 0.016f, -0.041f }, //RightHand,
	{ -0.079f, -0.017f, -0.022f }, //RightHandThumb1,
	{ -1, 0, 0 }, //RightHandThumb2,
	{ -1, 0, 0 }, //RightHandThumb3,
	{}, //RightHandThumb4, (doesn't matter)
	{ -0.132f, 0, -0.005f }, //RightHandIndex1,
	{ -0.083f, 0, -0.003f }, //RightHandIndex2,
	{ -0.061f, 0, -0.002f }, //RightHandIndex3,
	{}, //RightHandIndex4, (doesn't matter)
	{ -1, 0, 0 }, //RightHandMiddle1,
	{ -1, 0, 0 }, //RightHandMiddle2,
	{ -1, 0, 0 }, //RightHandMiddle3,
	{}, //RightHandMiddle4, (doesn't matter)
	{ -1, 0, 0 }, //RightHandRing1,
	{ -1, 0, 0 }, //RightHandRing2,
	{ -1, 0, 0 }, //RightHandRing3,
	{}, //RightHandRing4, (doesn't matter)
	{ -1, 0, 0 }, //RightHandPinky1,
	{ -1, 0, 0 }, //RightHandPinky2,
	{ -1, 0, 0 }, //RightHandPinky3,
	{}, //RightHandPinky4, (doesn't matter)
	{ 0, -1, 0 }, //LeftUpLeg,
	{ 0, -1, 0 }, //LeftLeg,
	{ 0, -0.196f, -0.405f }, //LeftFoot,
	{ 0, 0, -1 }, //LeftToeBase,
	{}, //LeftToeBaseEnd, (doesn't matter)
	{ 0, -1, 0 }, //RightUpLeg,
	{ 0, -1, 0 }, //RightLeg,
	{ 0, -0.196f, -0.405f }, //RightFoot,
	{ 0, 0, -1 }, //RightToeBase,
	{}, //RightToeBaseEnd, (doesn't matter)
};

HumanBone::HumanBoneId HumanBoneParents[HumanBone::NumHumanBones] =
{
	HumanBone::None, //Root,
	HumanBone::Root, //Hips,
	HumanBone::Hips, //Spine,
	HumanBone::Spine, //Spine1,
	HumanBone::Spine1, //Spine2,
	HumanBone::Spine2, //Neck,
	HumanBone::Neck, //Head,
	HumanBone::Head, //HeadTop,
	HumanBone::Spine2, //LeftShoulder,
	HumanBone::LeftShoulder, //LeftArm,
	HumanBone::LeftArm, //LeftForeArm,
	HumanBone::LeftForeArm, //LeftHand,
	HumanBone::LeftHand, //LeftHandThumb1,
	HumanBone::LeftHandThumb1, //LeftHandThumb2,
	HumanBone::LeftHandThumb2, //LeftHandThumb3,
	HumanBone::LeftHandThumb3, //LeftHandThumb4,
	HumanBone::LeftHand, //LeftHandIndex1,
	HumanBone::LeftHandIndex1, //LeftHandIndex2,
	HumanBone::LeftHandIndex2, //LeftHandIndex3,
	HumanBone::LeftHandIndex3, //LeftHandIndex4,
	HumanBone::LeftHand, //LeftHandMiddle1,
	HumanBone::LeftHandMiddle1, //LeftHandMiddle2,
	HumanBone::LeftHandMiddle2, //LeftHandMiddle3,
	HumanBone::LeftHandMiddle3, //LeftHandMiddle4,
	HumanBone::LeftHand, //LeftHandRing1,
	HumanBone::LeftHandRing1, //LeftHandRing2,
	HumanBone::LeftHandRing2, //LeftHandRing3,
	HumanBone::LeftHandRing3, //LeftHandRing4,
	HumanBone::LeftHand, //LeftHandPinky1,
	HumanBone::LeftHandPinky1, //LeftHandPinky2,
	HumanBone::LeftHandPinky2, //LeftHandPinky3,
	HumanBone::LeftHandPinky3, //LeftHandPinky4,
	HumanBone::Spine2, //RightShoulder,
	HumanBone::RightShoulder, //RightArm,
	HumanBone::RightArm, //RightForeArm,
	HumanBone::RightForeArm, //RightHand,
	HumanBone::RightHand, //RightHandThumb1,
	HumanBone::RightHandThumb1, //RightHandThumb2,
	HumanBone::RightHandThumb2, //RightHandThumb3,
	HumanBone::RightHandThumb3, //RightHandThumb4,
	HumanBone::RightHand, //RightHandIndex1,
	HumanBone::RightHandIndex1, //RightHandIndex2,
	HumanBone::RightHandIndex2, //RightHandIndex3,
	HumanBone::RightHandIndex3, //RightHandIndex4,
	HumanBone::RightHand, //RightHandMiddle1,
	HumanBone::RightHandMiddle1, //RightHandMiddle2,
	HumanBone::RightHandMiddle2, //RightHandMiddle3,
	HumanBone::RightHandMiddle3, //RightHandMiddle4,
	HumanBone::RightHand, //RightHandRing1,
	HumanBone::RightHandRing1, //RightHandRing2,
	HumanBone::RightHandRing2, //RightHandRing3,
	HumanBone::RightHandRing3, //RightHandRing4,
	HumanBone::RightHand, //RightHandPinky1,
	HumanBone::RightHandPinky1, //RightHandPinky2,
	HumanBone::RightHandPinky2, //RightHandPinky3,
	HumanBone::RightHandPinky3, //RightHandPinky4,
	HumanBone::Hips, //LeftUpLeg,
	HumanBone::LeftUpLeg, //LeftLeg,
	HumanBone::LeftLeg, //LeftFoot,
	HumanBone::LeftFoot, //LeftToeBase,
	HumanBone::LeftToeBase, //LeftToeBaseEnd,
	HumanBone::Hips, //RightUpLeg,
	HumanBone::RightUpLeg, //RightLeg,
	HumanBone::RightLeg, //RightFoot,
	HumanBone::RightFoot, //RightToeBase
	HumanBone::RightToeBase, //RightToeBaseEnd
};

uint32_t HumanBone::parent(uint32_t id)
{
	if (id >= NumHumanBones) return HumanBone::None;
	return HumanBoneParents[id];
}

uint32_t HumanBone::target(uint32_t id)
{
	if (id >= NumHumanBones) return HumanBone::None;
	return HumanBoneDirectionTargets[id];
}

const float3& HumanBone::direction(uint32_t id)
{
	if (id >= NumHumanBones) return HumanBoneDefaultDirections[HumanBone::Root];
	return HumanBoneDefaultDirections[id];
}

const char* HumanBone::name(uint32_t id)
{
	if (id >= NumHumanBones) return nullptr;
	return HumanBoneNames[id];
}

struct ModelFile;
bool CorrectHumanBoneRotation(const ModelFile & dstModel, const ModelFile & srcModel, quat& rotation, uint32_t humanBoneId);

void Basename(char* buf, size_t bufSize, const char* path)
{
	size_t start = 0;
	size_t end = SIZE_MAX;
	size_t i = 0;
	while (path[i])
	{
		if (path[i] == '\\' || path[i] == '/')
		{
			start = i + 1;
		}
		if (path[i] == '.')
		{
			end = i;
		}
		i++;
	}

	if (end == SIZE_MAX || end < start)
	{
		end = i;
	}

	strncpy_s(buf, bufSize, path + start, end - start);
}

void Directory(char* buf, size_t bufSize, const char* path)
{
	size_t start = 0;
	size_t end = 0;
	size_t i = 0;
	while (path[i])
	{
		if (path[i] == '\\' || path[i] == '/')
		{
			end = i;
		}
		i++;
	}

	if (end == start)
	{
		strncpy_s(buf, bufSize, "./", 3);
	}
	else
	{
		strncpy_s(buf, bufSize, path + start, end - start + 1);
	}
}

bool EndWith(const char* str1, const char* str2)
{
	size_t len1 = strlen(str1), len2 = strlen(str2);
	if (len1 < len2) return false;

	for (size_t i = 0; i < len2; i++)
	{
		if (str1[len1 - len2 + i] != str2[i]) return false;
	}

	return true;
}

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

uint32_t loadBoneHierarchy(aiNode* node, BoneTree& bones, BoneTable& table, uint32_t parentId = UINT16_MAX, uint32_t lastSibling = UINT16_MAX)
{
	uint32_t boneId = static_cast<uint32_t>(bones.size());
	bones.push_back(Bone());
	Bone& bone = bones[boneId];
	bone.id = boneId;
	bone.parent = parentId;
	bone.nextSibling = UINT16_MAX;

	if (lastSibling != UINT16_MAX)
	{
		bones[lastSibling].nextSibling = boneId;
	}

	if (node->mName.length > 0)
	{
		strncpy(bone.name, node->mName.C_Str(), 127);
		bone.name[127] = 0;

		table.insert(std::make_pair(node->mName.C_Str(), boneId));
	}

	CopyMatrix(bone.transform, node->mTransformation);
	bone.offsetMatrix = float4x4(1.0f);

	uint32_t firstChild = UINT16_MAX;
	uint32_t lastChild = UINT16_MAX;
	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		uint32_t id = loadBoneHierarchy(node->mChildren[i], bones, table, boneId, lastChild);
		if (firstChild == UINT16_MAX)
		{
			firstChild = id;
		}
		lastChild = id;
	}
	bones[boneId].firstChild = firstChild;

	return boneId;
}

struct ForSortingFrame {
	uint16_t usedTime;
	ModelAnimFrame frame;
};

bool SortingFrameComp(ForSortingFrame i, ForSortingFrame j) {
	if (i.usedTime == j.usedTime) {
		if (i.frame.GetJointIndex() == j.frame.GetJointIndex()) {
			if (i.frame.GetChannelType() == j.frame.GetChannelType()) {
				return i.frame.time < j.frame.time;
			}
			else {
				return i.frame.GetChannelType() < j.frame.GetChannelType();
			}
		}
		else {
			return i.frame.GetJointIndex() < j.frame.GetJointIndex();
		}
	}
	else {
		return i.usedTime < j.usedTime;
	}

}

struct ModelFile
{
	Assimp::Importer				importer;
	tofu::model::ModelHeader		header;
	uint32_t						numVertices;
	uint32_t						numIndices;
	std::vector<Mesh>				meshes;
	std::vector<uint8_t>			vertices;
	std::vector<uint16_t>			indices;
	BoneTree						bones;
	BoneTable						boneTable;
	std::vector<Animation>			anims;
	std::vector<ForSortingFrame>	frames;
	std::vector<ModelAnimFrame>		orderedFrames;

	std::vector<uint32_t>			humanBoneBindings;
	std::vector<quat>				humanBoneWorldR;
	std::vector<quat>				humanBoneLocalR;
	std::vector<quat>				humanBoneCorrectionLocalR;
	std::vector<float3>				humanBoneLocalT;
	std::vector<uint32_t>			bonesHumanBoneIds;
	std::vector<bool>				boneHasOffsetMatrices;
	float3							rootOffset;
	float							hipsHeight;


	int Init(const char* filename, bool withAnim = true)
	{
		importer.FreeScene();

		unsigned int flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace
			| aiProcess_LimitBoneWeights | aiProcess_ValidateDataStructure;

		bool bConvertToLeftHanded = true;
		bool bStructureOfArray = false;

		if (bConvertToLeftHanded)
		{
			flags |= aiProcess_ConvertToLeftHanded;
		}

		const aiScene* scene = importer.ReadFile(std::string(filename), flags);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			fprintf(stderr, "failed to read input file.\n%s\n", importer.GetErrorString());
			return __LINE__;
		}

		header = {};
		header.Magic = tofu::model::kModelFileMagic;
		header.Version = tofu::model::kModelFileVersion;
		header.StructOfArray = bStructureOfArray ? 1 : 0;
		header.HasIndices = 1;
		header.HasTangent = 1;
		header.NumTexcoordChannels = 0;

		header.NumMeshes = scene->mNumMeshes;
		header.NumBones = 0;
		header.NumAnimations = 0;

		// gathering bone information
		if (scene->mRootNode->mNumChildren > 0) {
			// load bone hierarchy	
			loadBoneHierarchy(scene->mRootNode, bones, boneTable);
			header.NumBones = static_cast<uint32_t>(bones.size());
			boneHasOffsetMatrices.resize(bones.size(), false);
		}

		if (scene->mNumAnimations > 0 && withAnim)
		{
			header.NumAnimations = scene->mNumAnimations;
			header.HasAnimation = 1;
		}

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

		if (header.NumTexcoordChannels > tofu::model::kModelFileMaxTexcoordChannels)
		{
			header.NumTexcoordChannels = tofu::model::kModelFileMaxTexcoordChannels;
		}

		// gathering vertices information
		if (header.NumMeshes > 0)
		{
			meshes.resize(header.NumMeshes);
		}

		for (uint32_t i = 0; i < header.NumMeshes; ++i)
		{
			const aiMesh* mesh = scene->mMeshes[i];
			tofu::model::ModelMesh& meshInfo = meshes[i];
			meshInfo.NumVertices = mesh->mNumVertices;
			meshInfo.NumIndices = mesh->mNumFaces * 3;

			numVertices += meshInfo.NumVertices;
			numIndices += meshInfo.NumIndices;
		}

		// align index data size to dword
		if (numIndices % 2 != 0)
		{
			numIndices += 1;
		}

		uint32_t vertexSize = header.CalculateVertexSize();
		uint32_t verticesDataSize = vertexSize * numVertices;
		vertices.resize(verticesDataSize);
		indices.resize(numIndices);

		// populate vertices data
		if (bStructureOfArray)
		{
			printf("not implemented yet.\n");
			return __LINE__;
		}
		else
		{
			uint8_t* ptr = &(vertices[0]);
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
						aiVector3D& bitan = mesh->mBitangents[j];

						float handedness = (nor ^ tan) * bitan < 0.0f ? -1.0f : 1.0f;

						*reinterpret_cast<float4*>(ptr) = float4{ tan.x, tan.y, tan.z, handedness };
						ptr += sizeof(float4);
					}

					if (header.NumBones != 0)
					{
						// reserve space for bone Ids and weights
						*reinterpret_cast<uint4*>(ptr) = uint4();
						ptr += sizeof(uint4);
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

					uint16_t boneId = iter->second;

					if (IsIdentity(bones[boneId].offsetMatrix))
					{
						CopyMatrix(bones[boneId].offsetMatrix, bone->mOffsetMatrix);
						boneHasOffsetMatrices[boneId] = true;
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

						uint4* boneIds = reinterpret_cast<uint4*>(meshBaseAddr
							+ vertexSize * vertexId
							+ sizeof(float) * (header.HasTangent ? 10 : 6));

						float4* weights = reinterpret_cast<float4*>(boneIds + 1);

						if (weights->x == 0.0f)
						{
							boneIds->x = boneId;
							weights->x = weight;
						}
						else if (weights->y == 0.0f)
						{
							boneIds->y = boneId;
							weights->y = weight;
						}
						else if (weights->z == 0.0f)
						{
							boneIds->z = boneId;
							weights->z = weight;
						}
						else if (weights->w == 0.0f)
						{
							boneIds->w = boneId;
							weights->w = weight;
						}
						else
						{
							printf("mesh %d vertex %d has more than 4 bones bound.\n", i, vertexId);
							return __LINE__;
						}
					}

				}
			}
		}

		// populate indices data
		{
			uint16_t* pIndices = &(indices[0]);

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

					*pIndices = a;
					pIndices++;
					*pIndices = b;
					pIndices++;
					*pIndices = c;
					pIndices++;
				}
			}
		}

		// gathering animation information
		for (uint32_t iAnim = 0; iAnim < header.NumAnimations; iAnim++)
		{
			aiAnimation* anim = scene->mAnimations[iAnim];

			Animation animation = {
				"",
				static_cast<float>(anim->mDuration),
				static_cast<float>(anim->mTicksPerSecond),
				frames.size(),
				0
			};

			strncpy(animation.name, anim->mName.C_Str(), 127);
			animation.name[127] = 0;

			for (uint32_t iChan = 0; iChan < anim->mNumChannels; iChan++)
			{
				aiNodeAnim* chan = anim->mChannels[iChan];
				std::string boneName(chan->mNodeName.C_Str());
				auto iter = boneTable.find(boneName);
				if (iter == boneTable.end())
				{
					printf("unable to find bone %s\n", boneName.c_str());
					return __LINE__;
				}

				uint16_t boneId = iter->second;
				uint32_t numT = chan->mNumPositionKeys;
				uint32_t numR = chan->mNumRotationKeys;
				uint32_t numS = chan->mNumScalingKeys;

				// translation keys
				for (uint32_t iFrame = 0; iFrame < numT; iFrame++)
				{
					aiVectorKey& key = chan->mPositionKeys[iFrame];
					aiVectorKey& sortKey = chan->mPositionKeys[iFrame < 2 ? 0 : iFrame - 2];

					ForSortingFrame temp;
					temp.usedTime = static_cast<uint16_t>(round(sortKey.mTime));

					ModelAnimFrame &frame = temp.frame;
					frame.time = static_cast<uint16_t>(round(key.mTime));
					frame.SetJointIndex(boneId);
					frame.SetChannelType(kChannelTranslation);
					frame.value.x = key.mValue.x;
					frame.value.y = key.mValue.y;
					frame.value.z = key.mValue.z;

					frames.push_back(temp);
				}

				// rotation keys
				for (uint32_t iFrame = 0; iFrame < numT; iFrame++)
				{
					aiQuatKey& key = chan->mRotationKeys[iFrame];
					aiQuatKey& sortKey = chan->mRotationKeys[iFrame < 2 ? 0 : iFrame - 2];

					ForSortingFrame temp;
					temp.usedTime = static_cast<uint16_t>(round(sortKey.mTime));

					ModelAnimFrame &frame = temp.frame;
					frame.time = static_cast<uint16_t>(round(key.mTime));
					frame.SetJointIndex(boneId);
					frame.SetChannelType(kChannelRotation);

					quat q;
					q.x = key.mValue.x;
					q.y = key.mValue.y;
					q.z = key.mValue.z;
					q.w = key.mValue.w;

					//CompressQuaternion(q, *reinterpret_cast<uint32_t*>(&frame.value.x));

					bool negativeW;
					CompressQuaternion(q, frame.value, negativeW);
					frame.SetSignedBit(negativeW);

					frames.push_back(temp);
				}

				// scale keys
				for (uint32_t iFrame = 0; iFrame < numT; iFrame++)
				{
					aiVectorKey& key = chan->mScalingKeys[iFrame];
					aiVectorKey& sortKey = chan->mScalingKeys[iFrame < 2 ? 0 : iFrame - 2];

					ForSortingFrame temp;
					temp.usedTime = static_cast<uint16_t>(round(sortKey.mTime));

					ModelAnimFrame &frame = temp.frame;
					frame.time = static_cast<uint16_t>(round(key.mTime));
					frame.SetJointIndex(boneId);
					frame.SetChannelType(kChannelScale);
					frame.value.x = key.mValue.x;
					frame.value.y = key.mValue.y;
					frame.value.z = key.mValue.z;

					frames.push_back(temp);
				}
			}

			animation.numFrames = frames.size() - animation.startFrames;
			anims.push_back(animation);

			std::sort(frames.begin() + animation.startFrames, frames.end(), SortingFrameComp);
		}

		orderedFrames.reserve(frames.size());

		for (auto &frame : frames) {
			orderedFrames.push_back(std::move(frame.frame));
		}

		frames.resize(0);

		header.NumAnimationFrames = static_cast<uint32_t>(orderedFrames.size());

		return 0;
	}

	int LoadAvatar(const char* filename)
	{
		if (bones.empty())
			return __LINE__;

		FILE* fp = fopen(filename, "rb");
		if (!fp) return 0;

		fseek(fp, 0, SEEK_END);
		size_t size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* buffer = (char*)malloc(size + 1);
		fread(buffer, size, 1, fp);
		fclose(fp);
		buffer[size] = 0;

		rapidjson::Document doc;
		doc.Parse(buffer);

		free(buffer);

		assert(!doc.HasParseError());

		// column-majored
		std::vector<float4x4> humanBoneWorldMatrices(HumanBone::NumHumanBones, float4x4(1.0f));
		std::vector<float4x4> humanBoneLocalMatrices(HumanBone::NumHumanBones, float4x4(1.0f));

		humanBoneLocalT.resize(HumanBone::NumHumanBones, float3(0, 0, 0));
		humanBoneWorldR.resize(HumanBone::NumHumanBones, quat(1, 0, 0, 0));
		humanBoneLocalR.resize(HumanBone::NumHumanBones, quat(1, 0, 0, 0));
		humanBoneBindings.resize(HumanBone::NumHumanBones, UINT32_MAX);
		bonesHumanBoneIds.resize(bones.size(), UINT32_MAX);

		for (uint32_t i = 0; i < HumanBone::NumHumanBones; i++)
		{
			const char* humanBoneName = HumanBone::name(i);

			if (doc.HasMember(humanBoneName))
			{
				const char* boneName = doc[humanBoneName].GetString();

				if (boneName[0] == '*') boneName = humanBoneName;

				auto iter = boneTable.find(boneName);
				if (iter != boneTable.end())
				{
					humanBoneBindings[i] = iter->second;
					bonesHumanBoneIds[iter->second] = i;

					quat rot;
					float3 scale, trans;

					if (boneHasOffsetMatrices[iter->second])
					{
						humanBoneWorldMatrices[i] = inverse(transpose(bones[iter->second].offsetMatrix));
					}
					else
					{
						uint32_t p = HumanBone::parent(i);
						if (p == HumanBone::None)
						{
							humanBoneWorldMatrices[i] = mat4(1.0f);
						}
						else
						{
							decompose(transpose(bones[iter->second].transform), trans, rot, scale);

							// column-majored
							humanBoneWorldMatrices[i] = humanBoneWorldMatrices[p] * translate(mat4(1.0f), trans);
						}
					}


					decompose(humanBoneWorldMatrices[i],
						trans, humanBoneWorldR[i], scale);

					continue;
				}
				return __LINE__;
			}
			else
			{
				uint32_t p = HumanBone::parent(i);
				if (p == HumanBone::None)
				{
					humanBoneWorldMatrices[i] = mat4(1.0f);
					humanBoneWorldR[i] = quat(1, 0, 0, 0);
				}
				else
				{
					humanBoneWorldMatrices[i] = humanBoneWorldMatrices[p];
					humanBoneWorldR[i] = humanBoneWorldR[p];
				}
			}
		}
		
		// get local rotation for each bone in human skeleton
		for (uint32_t i = 0; i < HumanBone::NumHumanBones; i++)
		{
			uint32_t p = HumanBone::parent(i);
			humanBoneLocalR[i] = humanBoneWorldR[i];
			humanBoneLocalMatrices[i] = humanBoneWorldMatrices[i];
			if (p != HumanBone::None)
			{
				humanBoneLocalR[i] = inverse(humanBoneWorldR[p]) * humanBoneLocalR[i];
				humanBoneLocalMatrices[i] = inverse(humanBoneWorldMatrices[p]) * humanBoneLocalMatrices[i];
			}

			{
				vec3 t, s;
				quat r;
				decompose(humanBoneLocalMatrices[i], t, r, s);
				humanBoneLocalT[i] = t;
			}
		}

		// get rootOffset and hipsHeight
		{
			quat r; float3 t, s;
			decompose(humanBoneWorldMatrices[HumanBone::Root], rootOffset, r, s);
			decompose(humanBoneLocalMatrices[HumanBone::Hips], t, r, s);
			hipsHeight = t.y;
		}

		humanBoneCorrectionLocalR.resize(HumanBone::NumHumanBones, quat(1, 0, 0, 0));

		//fp = fopen((std::string(filename) + ".txt").c_str(), "w");

		// adjust 
		for (uint32_t i = HumanBone::Hips; i < HumanBone::NumHumanBones; i++)
		{
			uint32_t p = HumanBone::parent(i);
			humanBoneWorldMatrices[i] = humanBoneWorldMatrices[p] * humanBoneLocalMatrices[i];

			uint32_t t = HumanBone::target(i);
			if (t == HumanBone::None)
			{
				continue;
			}

			if (humanBoneBindings[t] == UINT32_MAX) continue;

			// vectors in parent's coordinate system
			vec3 boneStart = humanBoneLocalMatrices[i] * vec4(0, 0, 0, 1);
			vec3 boneEnd = humanBoneLocalMatrices[i] * (humanBoneLocalMatrices[t] * vec4(0, 0, 0, 1));
			vec3 boneDir = normalize(boneEnd - boneStart);

			vec3 stdBoneDir = inverse(mat3(humanBoneWorldMatrices[p])) * normalize(HumanBone::direction(i));

			float cosDelta = dot(boneDir, stdBoneDir);
			if (cosDelta > 0.999f)
			{
				humanBoneCorrectionLocalR[i] = quat(1, 0, 0, 0);
			}
			else
			{
				float delta = acosf(cosDelta);
				vec3 rotateAxis = normalize(cross(boneDir, stdBoneDir));

				humanBoneCorrectionLocalR[i] = angleAxis(delta, rotateAxis);
			}

			quat rot;
			vec3 trans, scale;
			decompose(humanBoneLocalMatrices[i], trans, rot, scale);
			rot = humanBoneCorrectionLocalR[i] * rot;
			humanBoneLocalMatrices[i] = transform(trans, rot, scale);
			humanBoneWorldMatrices[i] = humanBoneWorldMatrices[p] * humanBoneLocalMatrices[i];

			//{
			//	quat q = humanBoneCorrectionLocalR[i];
			//	vec3 v = degrees(eulerAngles(q));
			//	fprintf(fp, "%20s: (%10.6f, %10.6f, %10.6f) <> (%10.6f, %10.6f, %10.6f)\n                      (%10.6f, %10.6f, %10.6f, %10.6f) = [%7.2f, %7.2f, %7.2f]\n",
			//		HumanBone::name(i),
			//		boneDir.x, boneDir.y, boneDir.z,
			//		stdBoneDir.x, stdBoneDir.y, stdBoneDir.z,
			//		q.w, q.x, q.y, q.z,
			//		v.x, v.y, v.z
			//	);
			//}
		}

		//fclose(fp);

		return 0;
	}

	int MergeAnimation(ModelFile& other)
	{
		if (!other.header.HasAnimation
			|| other.bones.empty()
			|| other.anims.empty())
			return 0;

		/*if (header.NumBones != other.header.NumBones)
		{
			printf("bone count doesn't match.\n");
			return __LINE__;
		}*/

		header.HasAnimation = 1;

		// override bone offset
		for (uint16_t i = 0; i < other.header.NumBones; i++)
		{
			ModelBone &bone = other.bones[i];
			std::string boneName(bone.name);

			auto iter = boneTable.find(boneName);
			if (iter != boneTable.end()) {
				uint16_t boneId = iter->second;
				bones[boneId].offsetMatrix = other.bones[i].offsetMatrix;
			}
		}

		//anims.resize(anims.size() + other.anims.size());
		anims.insert(anims.end(), other.anims.begin(), other.anims.end());

		// change frame id
		for (auto &frame : other.orderedFrames) {
			auto iter = boneTable.find(other.bones[frame.GetJointIndex()].name);

			if (iter != boneTable.end()) {
				frame.SetJointIndex(iter->second);
			}
			else {
				frame.SetJointIndex(kModelMaxJointIndex);
			}
		}

		orderedFrames.insert(orderedFrames.end(), other.orderedFrames.begin(), other.orderedFrames.end());

		for (uint32_t i = 0; i < other.header.NumAnimations; i++)
		{
			Animation& anim = anims[i + header.NumAnimations];
			anim.startFrames += header.NumAnimationFrames;
		}

		header.NumAnimations = static_cast<uint32_t>(anims.size());
		header.NumAnimationFrames = static_cast<uint32_t>(orderedFrames.size());

		return 0;
	}

	int RetargetAnimation(ModelFile& other)
	{
		if (humanBoneBindings.empty() || other.humanBoneBindings.empty() || other.anims.empty())
			return __LINE__;

		for (uint32_t i = 0; i < other.anims.size(); i++)
		{
			RetargetAnimation(other, i);
		}

		return 0;
	}

	int RetargetAnimation(ModelFile& other, const char* clip, const char* name = nullptr)
	{
		if (humanBoneBindings.empty() || other.humanBoneBindings.empty() || other.anims.empty())
			return __LINE__;

		for (uint32_t i = 0; i < other.anims.size(); i++)
		{
			if (strncmp(other.anims[i].name, clip, 127) == 0)
			{
				return RetargetAnimation(other, i, name);
			}
		}

		return __LINE__;
	}

	int RetargetAnimation(ModelFile& other, uint32_t animId, const char* name = nullptr)
	{
		if (humanBoneBindings.empty() || other.humanBoneBindings.empty() || other.anims.empty())
			return __LINE__;

		Animation& srcAnim = other.anims[animId];
		Animation dstAnim = {};

		if (nullptr == name)
		{
			name = srcAnim.name;
		}

		strcpy(dstAnim.name, name);

		dstAnim.tickCount = srcAnim.tickCount;
		dstAnim.ticksPerSecond = srcAnim.ticksPerSecond;

		header.HasAnimation = 1;

		std::vector<ModelAnimFrame> frames;
		frames.reserve(srcAnim.numFrames);

		for (uint32_t i = 0; i < srcAnim.numFrames; i++)
		{
			ModelAnimFrame frame = other.orderedFrames[srcAnim.startFrames + i];

			uint32_t srcBoneId = frame.GetJointIndex();
			uint32_t humanBoneId = other.bonesHumanBoneIds[srcBoneId];
			if (humanBoneId == UINT32_MAX) continue;

			uint32_t dstBoneId = humanBoneBindings[humanBoneId];
			if (dstBoneId == UINT32_MAX) continue;

			frame.SetJointIndex(dstBoneId);

			auto type = frame.GetChannelType();
			if (type == tofu::model::kChannelRotation)
			{
				quat q(1, 0, 0, 0);
				DecompressQuaternion(q, frame.value, frame.GetSignedBit());

				if (!CorrectHumanBoneRotation(*this, other, q, humanBoneId))
				{
					return __LINE__;
				}

				bool sign;
				CompressQuaternion(q, frame.value, sign);
				frame.SetSignedBit(sign);
			}
			else if (type == tofu::model::kChannelTranslation)
			{
				// apply t from dest T-pose except Hips and Root
				if (humanBoneId == HumanBone::Root)
				{
					frame.value = frame.value - other.rootOffset + rootOffset;
				}
				else if (humanBoneId == HumanBone::Hips)
				{
					frame.value.y = frame.value.y - other.hipsHeight + hipsHeight;
				}
				else
				{
					frame.value = humanBoneLocalT[humanBoneId];
				}
			}
			else if (type == tofu::model::kChannelScale)
			{
				//
			}

			frames.push_back(frame);
		}

		dstAnim.startFrames = orderedFrames.size();
		dstAnim.numFrames = frames.size();
		orderedFrames.insert(orderedFrames.end(), frames.begin(), frames.end());

		anims.push_back(dstAnim);

		header.NumAnimations = static_cast<uint32_t>(anims.size());
		header.NumAnimationFrames = static_cast<uint32_t>(orderedFrames.size());

		return 0;
	}

	int Write(const char* filename)
	{
		FILE* file = nullptr;
		if (0 != fopen_s(&file, filename, "wb") || nullptr == file)
		{
			printf("failed to create output file.\n");
			return __LINE__;
		}

		// writing header
		if (1 != fwrite(&header, sizeof(header), 1, file))
		{
			printf("failed to write header data.\n");
			return __LINE__;
		}

		// writing mesh data
		if (1 != fwrite(&meshes[0], sizeof(Mesh) * header.NumMeshes, 1, file))
		{
			printf("failed to write mesh data.\n");
			return __LINE__;
		}

		// write vertices to file
		if (1 != fwrite(&vertices[0], header.CalculateVertexSize() * numVertices, 1, file))
		{
			printf("failed to write vertices to the file.\n");
			return __LINE__;
		}

		// write indices to file
		if (1 != fwrite(&indices[0], sizeof(uint16_t) * numIndices, 1, file))
		{
			printf("failed to write indices to the file.\n");
			return __LINE__;
		}

		// bone data
		if (header.NumBones > 0)
		{
			if (1 != fwrite(&(bones[0]), sizeof(Bone) * header.NumBones, 1, file))
			{
				printf("failed to write bone list to the file.\n");
				return __LINE__;
			}
		}

		// animation data
		if (header.HasAnimation)
		{
			if (1 != fwrite(&(anims[0]), sizeof(Animation) * header.NumAnimations, 1, file))
			{
				printf("failed to write animation list to the file.\n");
				return __LINE__;
			}

			if (1 != fwrite(&(orderedFrames[0]), sizeof(ModelAnimFrame) * header.NumAnimationFrames, 1, file))
			{
				printf("failed to write frame list to the file.\n");
				return __LINE__;
			}
		}

		fclose(file);
		return 0;
	}

	bool HasTextures()
	{
		const aiScene* scene = importer.GetScene();
		return nullptr != scene && scene->HasTextures();
	}

	int WriteTextures(const char* basepath)
	{
		const aiScene* scene = importer.GetScene();

		if (nullptr != scene && scene->HasTextures())
		{
			char path[1024] = {};

			for (uint32_t i = 0; i < scene->mNumTextures; i++)
			{
				aiTexture* tex = scene->mTextures[i];
				if (tex->CheckFormat("png"))
				{
					sprintf_s(path, 1024, "%s_%u.png", basepath, i);
					FILE* f = nullptr;
					if (0 != fopen_s(&f, path, "wb") || nullptr == f)
					{
						return __LINE__;
					}

					if (1 != fwrite(tex->pcData, tex->mWidth, 1, f))
					{
						printf("failed to write texture to the file.\n");
						return __LINE__;
					}
				}
				else
				{
					printf("unsupported texture format.\n");
					return __LINE__;
				}
			}
		}

		return 0;
	}
};

bool CorrectHumanBoneRotation(const ModelFile & dstModel, const ModelFile & srcModel, quat& rotation, uint32_t humanBoneId)
{
	uint32_t srcBoneId = srcModel.humanBoneBindings[humanBoneId];
	if (srcBoneId == UINT32_MAX) return false;

	uint32_t dstBoneId = dstModel.humanBoneBindings[humanBoneId];
	if (dstBoneId == UINT32_MAX) return false;

	uint32_t parentId = srcModel.bones[srcBoneId].parent;
	uint32_t parentHumanBoneId = srcModel.bonesHumanBoneIds[parentId];
	if (parentHumanBoneId != UINT32_MAX)
	{
		quat TPoseLocalR = srcModel.humanBoneCorrectionLocalR[humanBoneId] * srcModel.humanBoneLocalR[humanBoneId];

		//quat deltaR = inverse(TPoseLocalR) * r;
		quat deltaR = rotation *  inverse(TPoseLocalR);

		quat srcParentWorldR = srcModel.humanBoneWorldR[parentHumanBoneId];
		deltaR = srcParentWorldR * deltaR * inverse(srcParentWorldR);
		//r = inverse(srcParentWorldR) * deltaR * srcParentWorldR;

		quat dstParentWorldR = dstModel.humanBoneWorldR[parentHumanBoneId];
		deltaR = inverse(dstParentWorldR) * deltaR * dstParentWorldR;
		//r = dstParentWorldR * r * inverse(dstParentWorldR);

		quat modelStdTPoseR = dstModel.humanBoneCorrectionLocalR[humanBoneId] * dstModel.humanBoneLocalR[humanBoneId];

		//r = model.humanBoneLocalR[humanBoneId] * r;
		rotation = deltaR * modelStdTPoseR;

	}
	return true;
}

int execute_config(const char* configFilename)
{
	FILE* configFile = fopen(configFilename, "rb");
	if (nullptr == configFile)
		return __LINE__;

	fseek(configFile, 0, SEEK_END);
	size_t fileSize = ftell(configFile);
	rewind(configFile);

	char* buffer = new char[fileSize + 1];
	if (1 != fread(buffer, fileSize, 1, configFile))
		return __LINE__;

	buffer[fileSize] = 0;

	rapidjson::Document config;
	config.Parse(buffer);
	delete[] buffer;

	if (config.HasParseError() || !config.IsArray())
	{
		printf("not valid config file\n");
		return __LINE__;
	}

	for (rapidjson::SizeType iTarget = 0; iTarget < config.Size(); iTarget++)
	{
		rapidjson::Value& target = config[iTarget];
		const char* outputFilename = target["target"].GetString();
		const char* meshFilename = target["source"].GetString();

		ModelFile model = {};
		int err = model.Init(meshFilename, false);
		if (err) return err;

		model.LoadAvatar(target["avatar"].GetString());

		if (target.HasMember("animations") && target["animations"].IsArray())
		{
			rapidjson::Value& animations = target["animations"];
			for (rapidjson::SizeType iAnim = 0; iAnim < animations.Size(); iAnim++)
			{
				rapidjson::Value& animation = animations[iAnim];
				ModelFile model2 = {};
				err = model2.Init(animation["source"].GetString());
				if (err) return err;

				model2.LoadAvatar(animation["avatar"].GetString());

				if (model.RetargetAnimation(model2, animation["clip"].GetString(), animation["name"].GetString()))
					return __LINE__;
			}
		}

		// write
		err = model.Write(outputFilename);
		if (err) return err;

		if (model.HasTextures())
		{
			char directory[1024] = {};
			char basename[1024] = {};

			Directory(directory, 1024, outputFilename);
			Basename(basename, 1024, outputFilename);

			strcat_s(directory, 1024, basename);

			err = model.WriteTextures(directory);
			if (err) return err;
		}
	}

	return 0;
}

int list_animations(const char* filename)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, 0);

	if (nullptr == scene)
	{
		printf("%s\n", importer.GetErrorString());
		return __LINE__;
	}

	printf("contains %d animations:\n");
	for (uint32_t i = 0; i < scene->mNumAnimations; i++)
	{
		printf("[%5d] %s\n", i, scene->mAnimations[i]->mName.C_Str());
	}

	return 0;
}

int main(int argc, char* argv[])
{
	//argc = 5;

	//char* tempArgv[6] =
	//{
	//	"",
	//	//"../../assets/archer.model",
	//	"../../assets/archer_test.model",
	//	"../../assets/archer_idle_renamed.fbx",
	//	//"../../assets/archer_idle.fbx",
	//	//"../../assets/archer_walking.fbx",
	//	//"../../assets/archer_jump.fbx",
	//	//"../../assets/archer_running.fbx",

	//	//"../../assets/soldier.model",
	//	//"../../assets/Soilder_LSJ.fbx",
	//	"../../assets/KB_Movement.fbx",
	//	"../../assets/KB_Punches.fbx",
	//};

	////argc = 3;
	////
	////char* tempArgv[6] =
	////{
	////	"",
	////	"../../cube.model",
	////	"../../assets/cube.fbx",
	////};

	////////char* tempArgv[6] =
	////////{
	////////	"",
	////////	"../../ground.model",
	////////	"../../assets/ground.fbx",
	////////};

	//argv = tempArgv;

	if (argc < 2)
	{
		printf("model_converter output_file input_file1 [input_file2 ...]\n");
		printf("model_converter config.json\n");
		return 0;
	}

	if (argc == 2)
	{
		if (EndWith(argv[1], ".json"))
		{
			return execute_config(argv[1]);
		}
		else
		{
			return list_animations(argv[1]);
		}
	}
	else
	{
		ModelFile model = {};
		int err = model.Init(argv[2]);
		if (err) return err;

		for (int i = 3; i < argc; i++)
		{
			ModelFile model2 = {};
			err = model2.Init(argv[i]);
			if (err) return err;

			model.MergeAnimation(model2);
		}

		// write
		err = model.Write(argv[1]);
		if (err) return err;

		if (model.HasTextures())
		{
			char directory[1024] = {};
			char basename[1024] = {};

			Directory(directory, 1024, argv[1]);
			Basename(basename, 1024, argv[1]);

			strcat_s(directory, 1024, basename);

			err = model.WriteTextures(directory);
			if (err) return err;
		}
	}

	return 0;
}