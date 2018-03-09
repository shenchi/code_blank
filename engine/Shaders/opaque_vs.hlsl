cbuffer InstanceConstants : register (b0)
{
	matrix	matWorld;
	matrix	matWorld_IT;
};

cbuffer FrameConstants : register (b1)
{
	matrix	matView;
	matrix	matProj;
	matrix	matViewInv;
	matrix	matProjInv;
	float4	cameraPos;
	float4	bufferSize;
};

cbuffer FrameConstants : register (b2) {
	matrix lightView;
	matrix lightProjection;
}

struct Input
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

struct V2F
{
	float4 position	: SV_POSITION;
	float3 worldPos	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float4 posForShadow : POSITION1;
	float2 uv		: TEXCOORD0;
	
};

V2F main(Input input)
{
	V2F output;

	matrix matMVP = mul(mul(matWorld, matView), matProj);

	output.position = mul(float4(input.position, 1), matMVP);
	output.worldPos = mul(float4(input.position, 1), matWorld).xyz;
	output.normal = mul(input.normal, (float3x3)matWorld_IT);
	output.tangent = float4(mul(input.tangent.xyz, (float3x3)matWorld_IT), input.tangent.w);
	//output.normal = mul(input.normal, (float3x3)matWorld);
	//output.tangent = float4(mul(input.tangent.xyz, (float3x3)matWorld), input.tangent.w);
	output.uv = input.uv;

	// Calculate the position of this vertex relative to the shadow-casting light
	matrix shadowWVP = mul(mul(matWorld, lightView), lightProjection);
	output.posForShadow = mul(float4(input.position, 1.0f), shadowWVP);

	return output;
}