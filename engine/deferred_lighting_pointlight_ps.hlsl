struct PointLight
{
	float4					color;
	float					intensity;
	float					range;
	float					_padding1;
	float					_padding2;
};

cbuffer PointLightTransforms : register (b0)
{
	float4x4				transforms[1024];
};

cbuffer FrameConstants : register (b1)
{
	float4x4				matView;
	float4x4				matProj;
	float4x4				matViewInv;
	float4x4				matProjInv;
	float4					cameraPos;
	float4					bufferSize;
	float4					leftTopRay;
	float4					rightTopRay;
	float4					leftBottomRay;
	float4					rightBottomRay;
	float4					perspectiveParams;
	float					padding3[4 * 9];
};

cbuffer PointLightParams : register (b2)
{
	PointLight				lights[1024];
	uint					numLights;
};

Texture2D gBuffer1 : register(t0);
Texture2D gBuffer2 : register(t1);
Texture2D gBuffer3 : register(t2);

SamplerState samp : register(s0);

#include "PBR.hlsli"

float4 main(float4 clipPos : SV_POSITION, uint iid : SV_InstanceID) : SV_TARGET
{
	float3 lightPos = transforms[iid][3].rgb;
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
	
	PointLight light = lights[iid];

	float3 lightDir = lightPos - worldPos.xyz;
	float dist = length(lightDir) / light.range;
	lightDir = normalize(lightDir);

	float NdotL = max(dot(lightDir, worldNormal), 0);
	float atten = (1 / (dist * dist)) * (1 - step(1, dist));
	atten *= (1 - dist);

	float3 H = normalize(lightDir + viewDir);
	float NdotH = max(dot(worldNormal, H), 0);
	float HdotV = max(dot(H, viewDir), 0);

	float NdotV = max(dot(worldNormal, viewDir), 0);

	float3 radiance = light.color.rgb * atten * light.intensity;
	float3 Lo = LightingModel(radiance, albedo, NdotH, HdotV, NdotV, NdotL, metallic, roughness);

	float3 finalColor = Lo;

	return float4(finalColor, 1);
}