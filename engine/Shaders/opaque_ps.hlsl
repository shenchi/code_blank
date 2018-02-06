//cbuffer LightingConstants : register (b0)
//{
//	float4	cameraPos;
//	float4  _reserv1[3];
//}
//
cbuffer DirectionalLightingConstants : register (b0)
{
	float4  lightColor[256];
	float4  lightDirection[256];
	float   count;
	float3	cameraPos;
	float4  lightPosition[256];
	float4   type[256];
	float4  _reserv1[3071];
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

	float4 allDiLightColor = float4(0, 0, 0, 0);
	float4 allPoLightColor = float4(0, 0, 0, 0);

	for (float i = 0.0f; i < count; i++) {
		if (type[i].x == 1.0f) {
			float3 direction = lightDirection[i].xyz;
			float lightAmount = saturate(dot(-direction, normal));
			allDiLightColor += lightColor[i] * lightAmount;
		}
		else if (type[i].x == 2.0f) {
			float3 position = lightPosition[i].xyz;
			float lightAmount = saturate(dot(normalize(position - input.worldPos), refl));
			allPoLightColor += lightColor[i] * lightAmount;
		}
		else if (type[i].x == 3.0f) {

		}

	}
		


	float3 color = diffuseTex.Sample(samp, input.uv).rgb;
	color = (allDiLightColor
		+ allPoLightColor)
		* color * 0.8 + cubeMap.Sample(samp, refl).rgb * 0.2;

	//color = sunLightColor * color;

	return float4(color, 1);
}
