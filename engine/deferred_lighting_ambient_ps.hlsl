struct DirectionalLight
{
	float4					direction;
	float4					color;
	float					intensity;
	float					_padding1;
	float					_padding2;
	float					_padding3;
};

cbuffer LightParametersDirectionalAndAmbient : register(b0)
{
	DirectionalLight		directionalLights[8];
	float4					ambient;
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
#include "PBR.hlsli"

Texture2D gBuffer1 : register(t0);
Texture2D gBuffer2 : register(t1);
Texture2D gBuffer3 : register(t2);
Texture2D gBuffer4 : register(t3);
TextureCube skyDiff : register(t4);
TextureCube skySpec : register(t5);
Texture2D BrdfLut : register(t6);

SamplerState samp : register(s0);
SamplerState sampForLUT : register(s1);

float4 main(float4 clipPos : SV_POSITION) : SV_TARGET
{
	float2 screenPos = clipPos.xy - 0.5;// / bufferSize.xy;

	int3 uv = int3(screenPos, 0);
	float4 albedo = gBuffer1.Load(uv).rgba;
	float4 texel = gBuffer2.Load(uv).rgba;
	float4 occlusion = gBuffer3.Load(uv).rgba;
	float3 emission = gBuffer4.Load(uv).rgb;

	float3 worldNormal = texel.rgb;
	float4 worldPos = float4(screenPos, texel.a, 1);
	float3 color = emission;
	uint numDirectionalLights = ambient.w;

	for (uint i = 0; i < numDirectionalLights; i++)
	{
		float NdotL = max(0, dot(worldNormal, -directionalLights[i].direction.xyz));
		color += NdotL * directionalLights[i].color.rgb * directionalLights[i].intensity * albedo.rgb;
	}

	float3 viewDir = normalize(cameraPos.xyz - worldPos.xyz);
	float ao = occlusion.x;
	float metallic = albedo.a;
	float roughness = 1 - occlusion.a;

	float3 ambientPBR = EnvironmentLight(worldNormal, viewDir, albedo.rgb, ao, metallic, roughness, skyDiff, skySpec, BrdfLut, samp, sampForLUT);
	color += ambientPBR * ambient.xyz;
	
	return float4(color, 1);
}
