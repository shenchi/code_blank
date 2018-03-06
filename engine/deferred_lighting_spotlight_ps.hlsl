cbuffer LightParameters : register (b0)
{
	float4x4				transform;
	float4x4				matView;
	float4x4				matProj;
	float4					direction;
	float4					color;
	float					range;
	float					intensity;
	float					spotAngle;
	float					padding[1 * 4 + 1];
};

Texture2D gBuffer1 : register(t0);
Texture2D gBuffer2 : register(t1);
Texture2D gBuffer3 : register(t2);
Texture2D shadowMap : register(t3);

SamplerState samp : register(s0);
SamplerState shadowSamp : register(s1);

float4 main(float4 clipPos : SV_POSITION) : SV_TARGET
{
	float3 lightPos = transform[3].rgb;
	float2 screenPos = clipPos.xy - 0.5;// / bufferSize.xy;

	float3 worldPos = gBuffer1.Load(int3(screenPos, 0)).rgb;
	float3 worldNormal = gBuffer2.Load(int3(screenPos, 0)).rgb;
	float3 albedo = gBuffer3.Load(int3(screenPos, 0)).rgb;

	float3 lightDir = lightPos - worldPos;
	float dist = length(lightDir);
	lightDir = normalize(lightDir);

	float NdotL = max(dot(lightDir, worldNormal), 0);
	float atten = max(0, 1 - (dist / range));

	float DdotL = dot(-direction.xyz, lightDir);

	float r = 1 - spotAngle;
	atten *= pow(max(0, 1 - (1 - DdotL) / r), 0.5);

	float value = NdotL * atten;

	float4 lightSpacePos = mul(mul(float4(worldPos, 1), matView), matProj);
	lightSpacePos /= lightSpacePos.w;
	lightSpacePos.xy = lightSpacePos.xy * 0.5 + 0.5;
	lightSpacePos.y = 1 - lightSpacePos.y;

	float currentDepth = lightSpacePos.z;
	//currentDepth += 0.0001f;
	float shadow = 0;
	for (int i = -3; i < 4; i++) {
		for (int j = -3; j < 4; j++) {
			float cloestDepth = shadowMap.Sample(shadowSamp, lightSpacePos.xy + float2(i, j) * 0.0009765625).r;
			shadow += (currentDepth > cloestDepth ? 1 : 0);
		}
	}
	shadow /= 25;

	return float4(shadow * value * color.rgb * albedo, 1);
}
