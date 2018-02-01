cbuffer InstanceConstants : register (b0)
{
	matrix	matWorld;
};

cbuffer FrameConstants : register (b1)
{
	matrix	matLightView;
	matrix	matLightProj;
};

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
};

V2F main(Input input)
{
	V2F output;

	matrix matMVPLightSpace = mul(mul(matWorld, matLightView), matLightProj);

	output.position = mul(float4(input.position, 1), matMVPLightSpace);
	return output;
}