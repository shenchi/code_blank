Texture2DArray textures : register (t0);
SamplerState samp : register (s0);

struct Input
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(Input input) : SV_TARGET
{
	float2 uv = input.uv;
	float3 color = textures.Sample(samp, float3(uv, 0)).rgb * 0.2;
	color += textures.Sample(samp, float3(uv, 1)).rgb * 0.2;
	color += textures.Sample(samp, float3(uv, 2)).rgb * 0.2;
	color += textures.Sample(samp, float3(uv, 3)).rgb * 0.2;
	color += textures.Sample(samp, float3(uv, 4)).rgb * 0.2;

	return float4(color, 1.0f);
}
