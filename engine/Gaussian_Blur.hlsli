Texture2D tex : register(t0);
SamplerState samp : register(s0);

//#define SAMPLE_COUNT 6
//static float offset[SAMPLE_COUNT] = { 0, 1.4845360824742266, 3.463917525773196, 5.443298969072165, 7.422680412371134, 9.402061855670103 };
//static float weight[SAMPLE_COUNT] = { 0.08386783725046117, 0.15938312254618253, 0.12993450638945953, 0.08989179687321099, 0.052709280893837356, 0.026147374672079025 };

#define SAMPLE_COUNT 3
static float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
static float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

struct Input
{
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD0;
};

float4 main(Input input) : SV_TARGET
{
	float2 texSize;
	tex.GetDimensions(texSize.x, texSize.y);
	float2 texelSize = 1.0f / texSize;

	float2 uv = input.position.xy / texSize;

#ifdef TOFU_BLUR_HORIZONTAL
	float2 step = float2(texelSize.x, 0);
#else
	float2 step = float2(0, texelSize.y);
#endif

	float3 color = tex.Sample(samp, uv).rgb * weight[0];

	for (int i = 1; i < SAMPLE_COUNT; i++)
	{
		color += tex.Sample(samp, uv + step * offset[i]).rgb * weight[i];
		color += tex.Sample(samp, uv - step * offset[i]).rgb * weight[i];
	}

	return float4(color, 1.0f);
}
