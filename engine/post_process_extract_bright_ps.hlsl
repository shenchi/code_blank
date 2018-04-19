static const float3 GrayScaleIntensity = { 0.299f, 0.587f, 0.114f };

Texture2D tex : register(t0);

float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
	float2 uv = pos.xy - 0.5;
	float3 color = tex.Load(int3(uv, 0)).rgb;

	float intensity = dot(color.rgb, GrayScaleIntensity);
	float bloomThreshold = 1;

	return float4(step(bloomThreshold, intensity) * color, 1);
}

