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
};


Texture2D diffuseTex : register(t0);
Texture2D normalMap : register(t1);
Texture2D metallicGlossMap : register(t2);
Texture2D occlusionMap : register(t3);

SamplerState samp : register(s0);

PS_OUTPUT main(V2F input)
{
	PS_OUTPUT output;

	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent.xyz - dot(input.tangent.xyz, normal) * normal);
	float3 bitangent = cross(tangent, normal) * input.tangent.w;

	float3x3 TBN = float3x3(tangent, bitangent, normal);

	float3 normTexel = normalMap.Sample(samp, input.uv).xyz * 2.0 - 1.0;

	normal = mul(normTexel, TBN);

	float3 albedo = diffuseTex.Sample(samp, input.uv).rgb;
	float2 metallicGloss = metallicGlossMap.Sample(samp, input.uv).ra;
	float3 occlusion = occlusionMap.Sample(samp, input.uv).rgb;

	albedo = pow(albedo, 2.2);

	output.rt1 = float4(albedo, metallicGloss.x);
	output.rt2 = float4(normal, input.position.z);
	output.rt3 = float4(occlusion, metallicGloss.y);

	return output;
}