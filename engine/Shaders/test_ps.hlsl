cbuffer LightingConstants : register (b0)
{
	float4  lightColor;
	float3  lightDirection;
	float   padding1;
	float3	lightPos;
	
}

struct PS_INPUT
{
	float4		position	: SV_POSITION;
	float2		texcoord	: TEXCOORD0;
};

Texture2D tex : register(t0);

SamplerState samp : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	//return float4(lightColor);
	return tex.Sample(samp, input.texcoord);
}