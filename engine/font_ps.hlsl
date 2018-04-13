struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD0;
};

Texture2D tex : register (t0);

SamplerState samp : register (s0);

float4 main(PSInput input) : SV_TARGET
{
	float v = tex.Sample(samp, input.texcoord).r;

	return input.color * v;
}