cbuffer InstanceConstants : register (b0)
{
	matrix		matModel;
}

cbuffer FrameConstants : register (b1)
{
	matrix		matView;
	matrix		matProj;
	float4		cameraPos;
};

struct VS_INPUT
{
	float4		position	: POSITION;
	float3		normal		: NORMAL;
	float4		tangent		: TANGENT;
	float2		texcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4		position	: SV_POSITION;
	float2		texcoord	: TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	//float4 pos = mul(mul(mul(input.position, matModel), matView), matProj);
	//float4 pos = mul(mul(input.position, matView), matProj);

	float4 pos = mul(input.position, matModel);
	pos = mul(pos, matView);
	pos = mul(pos, matProj);

	VS_OUTPUT output;
	output.position = pos;
	output.texcoord = input.texcoord;

	return output;
}
