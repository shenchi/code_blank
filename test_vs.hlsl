cbuffer InstanceConstants : register (b0)
{
	matrix		matModel;
}

cbuffer FrameConstants : register (b1)
{
	matrix		matView;
	matrix		matProj;
};

struct VS_INPUT
{
	float4		position	: POSITION;
	float3		normal		: NORMAL;
	float3		tangent		: TANGENT;
	float2		texcoord	: TEXCOORD0;
};

float4 main(VS_INPUT input) : SV_POSITION
{
	//float4 pos = mul(mul(mul(input.position, matModel), matView), matProj);
	//float4 pos = mul(mul(input.position, matView), matProj);

	float4 pos = mul(input.position, matModel);
	pos = mul(pos, matView);
	pos = mul(pos, matProj);

	return pos;
}
