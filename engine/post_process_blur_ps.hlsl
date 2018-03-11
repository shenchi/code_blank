Texture2D brightPart : register(t0);

float4 main(float4 pos : SV_POSITION) : SV_TARGET{

	float2 uv = pos.xy - 0.5;

	float3 totalColor = float3(0, 0, 0);

	float pixelWidth, pixelHeight;
	brightPart.GetDimensions(pixelWidth, pixelHeight);
	pixelWidth = 1 / pixelWidth;
	pixelHeight = 1 / pixelHeight;
	uint numSamples = 0;
	int blurAmount = 5;
	for (int y = -blurAmount; y <= blurAmount; y++)
	{
		for (int x = -blurAmount; x <= blurAmount; x++)
		{
			totalColor += brightPart.Load(int3(uv + float2(x * pixelWidth,y * pixelHeight), 0)).rgb;
			numSamples++;
		}
	}

	float3 color = totalColor / numSamples;

	return float4(color, 1);
}