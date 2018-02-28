cbuffer LightParameters : register (b0)
{
	float4x4				transform;
	float4					color;
	float					range;
	float					intensity;
	float					spotAngle;
	float					padding[10 * 4 + 1];
};

cbuffer FrameConstants : register (b1)
{
	matrix	matView;
	matrix	matProj;
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
	matrix matMVP = mul(mul(transform, matView), matProj);
	return mul(float4(input.position, 1.0f), matMVP);
}