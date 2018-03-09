cbuffer InstanceConstants : register (b0)
{
	matrix	matWorld;
	matrix	matWorld_IT;
};

cbuffer LightParameters : register (b1)
{
	float4x4	transform;
	float4x4	matView;
	float4x4	matProj;
	float4		direction;
	float4		color;
	float		range;
	float		intensity;
	float		spotAngle;
	float		padding[1 * 4 + 1];
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
