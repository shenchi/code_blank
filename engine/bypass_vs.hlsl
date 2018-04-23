struct Input
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

struct Output
{
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD0;
};

Output main(Input input)
{
	Output output;
	output.position = float4(input.position, 1);
	output.uv = input.uv;
	return output;
}
