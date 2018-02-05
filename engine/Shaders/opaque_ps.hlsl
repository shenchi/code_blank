//cbuffer LightingConstants : register (b0)
//{
//	float4	cameraPos;
//	float4  _reserv1[3];
//}
//
cbuffer LightingConstants : register (b0)
{
	float4  lightColor;
	float3  lightDirection;
	float   padding1;
	float3	lightPos;
	float   padding2;
	float3   cameraPos;
	float   padding3;
	float4  _reserv1[12];

}

struct V2F
{
	float4 position	: SV_POSITION;
	float3 worldPos	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

TextureCube cubeMap : register(t0);
Texture2D diffuseTex : register(t1);
Texture2D normalMap : register(t2);
SamplerState samp : register(s0);

float4 main(V2F input) : SV_TARGET
{
	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent.xyz - dot(input.tangent.xyz, normal) * normal);
	float3 bitangent = cross(tangent, normal) * input.tangent.w;

	float3x3 TBN = float3x3(tangent, bitangent, normal);

	float3 normTexel = normalMap.Sample(samp, input.uv).xyz * 2.0 - 1.0;

	normal = mul(normTexel, TBN);

	//return float4(input.uv, 0, 1);
	//return float4(normal, 1);

	float3 viewDir = normalize(cameraPos.xyz - input.worldPos);
	float3 refl = reflect(-viewDir, normal);

	float sunLightAmount = saturate(dot(-lightDirection, normal));
	float4 sunLightColor = lightColor * sunLightAmount;

	float3 color = diffuseTex.Sample(samp, input.uv).rgb;

	return float4(color, 1);
	color = sunLightColor * color *0.8 + cubeMap.Sample(samp, refl).rgb * 0.2;
	//color =  color * 0.8 + cubeMap.Sample(samp, refl).rgb * 0.2;

	//color = sunLightColor * color;

	return float4(color, 1);
}