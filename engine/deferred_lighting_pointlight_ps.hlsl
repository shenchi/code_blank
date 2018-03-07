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

SamplerState samp : register(s0);

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
	float atten = 1 / (dist * dist) * (1 - step(1, dist));

	float3 H = normalize(lightDir + viewDir);
	float NdotH = max(dot(worldNormal, H), 0);
	float HdotV = max(dot(H, viewDir), 0);

	float NdotV = max(dot(worldNormal, viewDir), 0);

	float value = NdotL * atten;

	float3 radiance = color.rgb * atten * intensity;
	float3 Lo = LightingModel(radiance, albedo, NdotH, HdotV, NdotV, NdotL, metallic, roughness);

	float3 finalColor = Lo;

	return float4(finalColor, 1);
}