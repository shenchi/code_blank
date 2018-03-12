cbuffer InstanceConstants : register (b0)
{
	matrix	matWorld;
	matrix	matWorld_IT;
};

cbuffer ShadowMatrices : register (b1)
{
	float4x4	matView;
	float4x4	matProj;
	float4x4	padding1;
	float4x4	padding2;
};

cbuffer BoneMatrices : register (b2)
{
	matrix bones[256];
};

struct Input
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	uint4  boneIds	: BLENDINDICES;
	float4 weights	: BLENDWEIGHT;
	float2 uv		: TEXCOORD0;
};

float4 main(Input input) : SV_POSITION
{
	matrix bone = bones[input.boneIds.x] * input.weights.x;
	bone += bones[input.boneIds.y] * input.weights.y;
	bone += bones[input.boneIds.z] * input.weights.z;
	bone += bones[input.boneIds.w] * input.weights.w;
	
	matrix matM = mul(bone, matWorld);
	matrix matMVP = mul(mul(matM, matView), matProj);
	return mul(float4(input.position, 1), matMVP);
}
