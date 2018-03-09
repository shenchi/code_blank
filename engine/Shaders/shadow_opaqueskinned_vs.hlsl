cbuffer InstanceConstants : register (b0)
{
	matrix	matWorld;
};

cbuffer FrameConstants : register (b1)
{
	matrix	matLightView;
	matrix	matLightProj;
};

cbuffer BoneMatrices : register (b2)
{
	matrix bones[256];
}

struct Input
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	uint4  boneIds	: BLENDINDICES;
	float4 weights	: BLENDWEIGHT;
	float2 uv		: TEXCOORD0;
};

struct V2F
{
	float4 position	: SV_POSITION;
};

V2F main(Input input)
{
	V2F output;

	matrix bone = bones[input.boneIds.x] * input.weights.x;
	bone += bones[input.boneIds.y] * input.weights.y;
	bone += bones[input.boneIds.z] * input.weights.z;
	bone += bones[input.boneIds.w] * input.weights.w;

	matrix matM = mul(bone, matWorld);

	matrix matMVPLightSpace = mul(mul(matM, matLightView), matLightProj);
	//matrix matMVPLightSpace = mul(mul(matWorld, matLightView), matLightProj);

	output.position = mul(float4(input.position, 1), matMVPLightSpace);
	return output;
}