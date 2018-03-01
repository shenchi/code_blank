cbuffer LightParameters : register (b0)
{
	float4x4				transform;
	float4					color;
	float					range;
	float					intensity;
	float					spotAngle;
	float					padding[10 * 4 + 1];
};

cbuffer ScreenParameters : register (b1)
{
	float4					bufferSize;
}

Texture2D gBuffer1 : register(t0);
Texture2D gBuffer2 : register(t1);
Texture2D gBuffer3 : register(t2);

SamplerState samp : register(s0);

float4 main(float4 clipPos : SV_POSITION) : SV_TARGET
{
	float3 lightPos = transform[3].rgb;
	float2 screenPos = clipPos.xy - 0.5;// / bufferSize.xy;
	
	float3 worldPos = gBuffer1.Load(int3(screenPos, 0)).rgb;
	float3 worldNormal = gBuffer2.Load(int3(screenPos, 0)).rgb;

	float3 lightDir = normalize(lightPos - worldPos);
	float NdotL = max(dot(lightDir, worldNormal), 0);

	return float4(NdotL.xxx, 1);
}