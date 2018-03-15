Texture2D tex : register(t0);

float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
	float2 uv = pos.xy - 0.5;

	float3 color = tex.Load(int3(uv, 0)).rgb;
	color = max(color, 0);
	
	color = color / (color + 1);

	//color = pow(color, 1 / 2.2);

	return float4(color, 1.0f);
}
