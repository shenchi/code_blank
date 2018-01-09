cbuffer InstanceConstants : register (b0)
{
	matrix	matWorld;
	//matrix	matWorld_IT;
};

cbuffer FrameConstants : register (b1)
{
	matrix	matView;
	matrix	matProj;
};

cbuffer BoneMatrices : register (b2)
{
	matrix bones[1024];
}

struct Input
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
	float4 boneIds	: BLENDINDICES;
	float4 weights	: BLENDWEIGHT;
	float2 uv		: TEXCOORD0;
};

struct V2F
{
	float4 position	: SV_POSITION;
	float3 worldPos	: POSITION;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

V2F main(Input input)
{
	V2F output;

	matrix bone = bones[input.boneIds.x] * input.weights.x;
	bone += bones[input.boneIds.y] * input.weights.y;
	bone += bones[input.boneIds.z] * input.weights.z;
	bone += bones[input.boneIds.w] * input.weights.w;

	matrix matM = mul(bone, matWorld);
	//matrix matM = matWorld;

	matrix matMVP = mul(mul(matM, matView), matProj);

	output.position = mul(float4(input.position, 1), matMVP);
	output.worldPos = mul(float4(input.position, 1), matM).xyz;
	//output.normal = mul(input.normal, (float3x3)matWorld_IT);
	//output.tangent = mul(input.tangent, (float3x3)matWorld_IT); 
	output.normal = mul(input.normal, (float3x3)matWorld);
	output.tangent = mul(input.tangent, (float3x3)matWorld);
	output.uv = input.uv;

	return output;
}