cbuffer LightParameters : register (b0)
{
	float4x4				transform;
	float4x4				matLightView;
	float4x4				matLightProj;
	float4					direction;
	float4					color;
	float					range;
	float					intensity;
	float					spotAngle;
	float					padding[1 * 4 + 1];
};

cbuffer FrameConstants : register (b1)
{
	matrix	matView;
	matrix	matProj;
	matrix	matViewInv;
	matrix	matProjInv;
	float4	cameraPos;
	float4	bufferSize;
};

Texture2D gBuffer1 : register(t0);
Texture2D gBuffer2 : register(t1);
Texture2D gBuffer3 : register(t2);
Texture2D shadowMap : register(t3);

SamplerState samp : register(s0);
SamplerState shadowSamp : register(s1);

#include "PBR.hlsli"

float4 main(float4 clipPos : SV_POSITION) : SV_TARGET
{
	float3 lightPos = transform[3].rgb;
	float2 screenPos = clipPos.xy - 0.5;

	float4 texel = gBuffer2.Load(int3(screenPos, 0)).rgba;
	float4 worldPos = float4(screenPos, texel.a, 1);
	worldPos.xy = (worldPos.xy / bufferSize.xy) * 2 - 1;
	worldPos.y = -worldPos.y;
	worldPos = mul(worldPos, matProjInv);
	worldPos /= worldPos.w;
	worldPos = mul(worldPos, matViewInv);

	float3 worldNormal = texel.rgb;

	texel = gBuffer1.Load(int3(screenPos, 0)).rgba;
	float3 albedo = texel.rgb;
	float metallic = texel.a;

	texel = gBuffer3.Load(int3(screenPos, 0)).rgba;
	float3 occlution = texel.rgb;
	float roughness = 1 - texel.a; // gloss

	float3 viewDir = normalize(cameraPos.xyz - worldPos.xyz);
	
	float3 lightDir = lightPos - worldPos.xyz;
	float dist = length(lightDir) / range;
	lightDir = normalize(lightDir);

	float NdotL = max(dot(lightDir, worldNormal), 0);
	float atten = 1 / (dist * dist);

	float3 H = normalize(lightDir + viewDir);
	float NdotH = max(dot(worldNormal, H), 0);
	float HdotV = max(dot(H, viewDir), 0);

	float NdotV = max(dot(worldNormal, viewDir), 0);

	// * for spot light - begin
	float DdotL = dot(-direction.xyz, lightDir);

	float r = 1 - spotAngle;
	atten *= pow(max(0, 1 - (1 - DdotL) / r), 2.0);
	// * for spot light - end

	// pbr
	float3 radiance = color.rgb  * atten * intensity;
	float3 Lo = LightingModel(radiance, albedo, NdotH, HdotV, NdotV, NdotL, metallic, roughness);

	// shadow
	float4 lightSpacePos = mul(mul(worldPos, matLightView), matLightProj);
	lightSpacePos /= lightSpacePos.w;
	lightSpacePos.xy = lightSpacePos.xy * 0.5 + 0.5;
	lightSpacePos.y = 1 - lightSpacePos.y;

	float currentDepth = lightSpacePos.z;
	float bias = max(0.001 * (1.0 - NdotL), 0.0001);
	currentDepth -= 0.00001;
	float shadow = 0;

	for (int i = -3; i < 4; i++) {
		for (int j = -3; j < 4; j++) {
			float cloestDepth = shadowMap.Sample(shadowSamp, lightSpacePos.xy + float2(i, j) * 0.0009765625).r;
			shadow += (currentDepth > cloestDepth ? 1 : 0);
		}
	}
	shadow /= 25;

	float3 finalColor = max(Lo, 0) * max(1 - shadow, 0);

	return float4(finalColor, 1);
}
