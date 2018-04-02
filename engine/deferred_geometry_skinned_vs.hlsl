cbuffer InstanceConstants : register (b0)
{
	matrix	matWorld;
	matrix	matWorld_IT;
};

cbuffer FrameConstants : register (b1)
{
	float4x4		matView;
	float4x4		matProj;
	float4x4		matViewInv;
	float4x4		matProjInv;
	float4			cameraPos;
	float4			bufferSize;
	float4			leftTopRay;
	float4			rightTopRay;
	float4			leftBottomRay;
	float4			rightBottomRay;
	float4			perspectiveParams;
	float			padding3[4 * 9];
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
	float3 worldPos	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
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
	float3x3 matM_IT = mul((float3x3)bone, (float3x3)matWorld_IT);

	matrix matMVP = mul(mul(matM, matView), matProj);

	output.position = mul(float4(input.position, 1), matMVP);
	output.worldPos = mul(float4(input.position, 1), matM).xyz;
	output.normal = normalize(mul(input.normal, matM_IT));
	output.tangent = float4(normalize(mul(input.tangent.xyz, matM_IT)), input.tangent.w);
	//output.normal = mul(input.normal, (float3x3)matWorld);
	//output.tangent = float4(mul(input.tangent.xyz, (float3x3)matWorld), input.tangent.w);
	output.uv = input.uv;

	return output;
}
