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


struct Input
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

float4 main(Input input) : SV_POSITION
{
	matrix matMVP = mul(mul(matWorld, matView), matProj);
	return mul(float4(input.position, 1), matMVP);
}
