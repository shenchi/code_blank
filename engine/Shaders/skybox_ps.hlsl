struct V2F
{
	float4 position	: SV_POSITION;
	float3 uv		: TEXCOORD0;
};

TextureCube cubeMap : register(t0);
SamplerState samp : register(s0);

float4 main(V2F input) : SV_TARGET
{
	return pow(cubeMap.Sample(samp, input.uv), 2.2);
}