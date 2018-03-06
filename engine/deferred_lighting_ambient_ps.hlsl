cbuffer LightParametersDirectionalAndAmbient : register(b0)
{
	float4					ambient;
	struct
	{
		float4				direction;
		float4				color;
	}						directionalLights[8];
};

Texture2D gBuffer1 : register(t0);
Texture2D gBuffer2 : register(t1);
Texture2D gBuffer3 : register(t2);

SamplerState samp : register(s0);

float4 main(float4 clipPos : SV_POSITION) : SV_TARGET
{
	float2 screenPos = clipPos.xy - 0.5;// / bufferSize.xy;

	//float3 worldPos = gBuffer1.Load(int3(screenPos, 0)).rgb;
	float3 worldNormal = gBuffer2.Load(int3(screenPos, 0)).rgb;
	float3 albedo = gBuffer3.Load(int3(screenPos, 0)).rgb;
	float3 color = albedo * ambient.rgb;

	uint numDirectionalLights = ambient.w;

	for (uint i = 0; i < numDirectionalLights; i++)
	{
		float NdotL = max(0, dot(worldNormal, -directionalLights[i].direction.xyz));
		color += NdotL * directionalLights[i].color.rgb * albedo;
	}
	
	return float4(color, 1);
}
