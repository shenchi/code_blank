struct VS_INPUT
{
	float4		position	: POSITION;
	float3		normal		: NORMAL;
	float3		tangent		: TANGENT;
	float2		texcoord	: TEXCOORD0;
};

float4 main(VS_INPUT input) : SV_POSITION
{
	float4 pos = input.position;
	pos.z = 0.5;
	return pos;
}
