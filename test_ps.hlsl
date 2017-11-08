struct PS_INPUT
{
	float4		position	: SV_POSITION;
	float2		texcoord	: TEXCOORD0;
};

Texture2D tex : register(t0);

SamplerState samp : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	return tex.Sample(samp, input.texcoord);
}