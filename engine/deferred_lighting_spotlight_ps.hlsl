cbuffer LightParameters : register (b0)
{
	float4x4				transform;
	float4					direction;
	float4					color;
	float					range;
	float					intensity;
	float					spotAngle;
	float					padding[9 * 4 + 1];
};

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

	float3 lightDir = lightPos - worldPos;
	float dist = length(lightDir);
	lightDir = normalize(lightDir);

	float NdotL = max(dot(lightDir, worldNormal), 0);
	float atten = max(0, 1 - (dist / range));

	float DdotL = dot(-direction.xyz, lightDir);

	float r = 1 - spotAngle;
	atten *= pow(max(0, 1 - (1 - DdotL) / r), 0.5);

	float value = NdotL * atten;

	return float4(value * color.rgb, 1);
}
