Texture2D brightPart : register(t0);

float4 main(float4 pos : SV_POSITION) : SV_TARGET{

	float2 uv = pos.xy - 0.5;
	float3 totalColor = float3(0, 0, 0);
	uint numSamples = 0;
	int blurAmount = 1;
	for (int y = -blurAmount; y <= blurAmount; y++)
	{
		for (int x = -blurAmount; x <= blurAmount; x++)
		{
			totalColor += brightPart.Load(int3(uv + int2(x ,y), 0)).rgb;
			numSamples++;
		}
	}

	float3 color = totalColor / numSamples;
	return float4(color, 1);
}