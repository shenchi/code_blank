struct Input
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

float4 main(Input input) : SV_POSITION
{
	return float4(input.position, 1);
}
