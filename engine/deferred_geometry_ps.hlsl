struct V2F
{
	float4 position	: SV_POSITION;
	float3 worldPos	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;

};

struct PS_OUTPUT
{
	float4		rt1	: SV_TARGET0;
	float4		rt2	: SV_TARGET1;
	float4		rt3	: SV_TARGET2;
	float4		rt4	: SV_TARGET3;
};

cbuffer MaterialParams : register (b0)
{
	float4		color;
	float4		emissionColor;
	float4		textureParams;
}

Texture2D diffuseTex : register(t0);
Texture2D normalMap : register(t1);
Texture2D metallicGlossMap : register(t2);
Texture2D occlusionMap : register(t3);
Texture2D emissionMap : register(t4);

SamplerState samp : register(s0);

PS_OUTPUT main(V2F input)
{
	PS_OUTPUT output;

	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent.xyz - dot(input.tangent.xyz, normal) * normal);
	float3 bitangent = cross(tangent, normal) * input.tangent.w;

	float3x3 TBN = float3x3(tangent, bitangent, normal);

	float2 uv = (input.uv * textureParams.xy) + textureParams.zw;

	float3 normTexel = normalMap.Sample(samp, uv).xyz * 2.0 - 1.0;

	normal = mul(normTexel, TBN);

	float3 albedo = diffuseTex.Sample(samp, uv).rgb;
	float2 metallicGloss = metallicGlossMap.Sample(samp, uv).ra;
	float3 occlusion = occlusionMap.Sample(samp, uv).rgb;
	float3 emission = emissionMap.Sample(samp, uv).rgb;

	//albedo = pow(albedo, 2.2);

	albedo *= color.rgb;

	emission *= emissionColor.rgb;

	output.rt1 = float4(albedo, metallicGloss.x);
	output.rt2 = float4(normal, input.position.z);
	output.rt3 = float4(occlusion, metallicGloss.y);
	output.rt4 = float4(emission, 0);

	return output;
}