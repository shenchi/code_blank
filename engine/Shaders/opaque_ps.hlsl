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
	float4  type[256];
	float4  _reserv1[3071];
}

struct V2F
{
	float4 position	: SV_POSITION;
	float3 worldPos	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float4 posForShadow : POSITION1;
	float2 uv		: TEXCOORD0;
	
};

TextureCube cubeMap : register(t0);
Texture2D diffuseTex : register(t1);
Texture2D normalMap : register(t2);
Texture2D shadowMap		: register(t3);
Texture2D roughnessMap  : register(t4);
Texture2D metallicMap   : register(t5);
Texture2D aoMap         : register(t6);
TextureCube irradianceMap: register(t7);
TextureCube prefilterMap: register(t8);
Texture2D brdfLUT: register(t9);
SamplerState samp : register(s0);
SamplerState shadowSampler : register(s1);
SamplerState samplerForLUT  : register(s2);
static float PI = 3.14159265359f;

float DistributionGGX(float3 N, float3 H, float roughness) {
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0);
	float NdotH2 = NdotH*NdotH;

	float nom = a2;
	float denom = NdotH2*(a2 - 1.0) + 1.0;
	denom = PI * denom*denom;

	return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = roughness + 1.0;  //direct lighting
	float k = r*r / 8.0;

	float nom = NdotV;
	float denom = NdotV*(1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
	float NdotV = max(dot(N, V), 0);
	float NdotL = max(dot(N, L), 0);

	float ggx1 = GeometrySchlickGGX(NdotV, roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1*ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0) {
	return F0 + (1.0 - F0)*pow(1.0 - cosTheta, 5.0);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness) {
	return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 getNormalFromNormalMap(V2F input) {
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	//---------------------------------------------------------------------------------------------
	//Normal Mapping

	//Read and unpack the normal from the map
	float3 normalFromMap = normalMap.Sample(samp, input.uv).xyz * 2 - 1;

	//Transform from tangent to world space
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);

	float3x3 TBN = float3x3(T, B, N);
	input.normal = normalize(mul(normalFromMap, TBN));
	return input.normal;
}


float4 main(V2F input) : SV_TARGET
{
	float4 albedo;
    float metallic;
    float roughness;
    float ao;
	float3 normal;

	float3 albedof = pow(diffuseTex.Sample(samp, input.uv).rgb, 2.2);
	albedo = float4(albedof, 1);
	input.normal = getNormalFromNormalMap(input);
	metallic = metallicMap.Sample(samp, input.uv).r;
	roughness = roughnessMap.Sample(samp, input.uv).r;
	ao = aoMap.Sample(samp, input.uv).r;
/*
	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent.xyz - dot(input.tangent.xyz, normal) * normal);
	float3 bitangent = cross(tangent, normal) * input.tangent.w;

	float3x3 TBN = float3x3(tangent, bitangent, normal);

	float3 normTexel = normalMap.Sample(samp, input.uv).xyz * 2.0 - 1.0;

	normal = mul(normTexel, TBN);

	float3 viewDir = normalize(cameraPos.xyz - input.worldPos);
	float3 refl = reflect(-viewDir, normal);*/

	float3 N = normalize(input.normal);
	float3 V = normalize(cameraPos - input.worldPos);
	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, metallic);

	float3 allDiLight = float3(0, 0, 0);
	float3 allPoLight = float3(0, 0, 0);
	float3 Lo = float3(0, 0, 0);

	for (float i = 0.0f; i < count; i++) {
		if (type[i].x == 1.0f) {
			//calculate per-light radiance
			float3 direction = lightDirection[i].xyz;
			float3 L = normalize(-direction);
			float3 H = normalize(L + V);
			float distance = 1;
			float attenuation = 1 / (distance*distance);
			float3 radiance = lightColor[i].xyz * attenuation;

			//cook-torrance BRDF
			float NDF = DistributionGGX(N, H, roughness);
			float3 F = fresnelSchlick(max(dot(N, V), 0), F0);
			float G = GeometrySmith(N, V, L, roughness);

			float3 kS = F;
			float3 kD = float3(1, 1, 1) - kS;
			kD *= 1 - metallic;

			float3 nominator = NDF * F * G;
			float denominator = 4 * max(dot(V, N), 0)*max(dot(L, N), 0) + 0.001; //avoid divide 0
			float3 specular = nominator / denominator;

			float NdotL = max(dot(N, L), 0);
			allDiLight += (kD*albedo / PI + specular)*radiance*NdotL;
		}
		else if (type[i].x == 2.0f) {
			//calculate per-light radiance
			float3 position = lightPosition[i].xyz;
			float3 L = normalize(position - input.worldPos);
			float3 H = normalize(L + V);
			float distance = length(position - input.worldPos);
			float attenuation = 1 / (distance*distance);
			float3 radiance = lightColor[i].xyz*attenuation;

			//cook-torrance BRDF
			float NDF = DistributionGGX(N, H, roughness);
			float3 F = fresnelSchlick(max(dot(N, V), 0), F0);
			float G = GeometrySmith(N, V, L, roughness);

			float3 kS = F;
			float3 kD = float3(1, 1, 1) - kS;
			kD *= 1 - metallic;

			float3 nominator = NDF * F * G;
			float denominator = 4 * max(dot(V, N), 0)*max(dot(L, N), 0) + 0.001; //avoid divide 0
			float3 specular = nominator / denominator;

			float NdotL = max(dot(N, L), 0);
			allPoLight += (kD*albedo / PI + specular)*radiance*NdotL;
		}
		else if (type[i].x == 3.0f) {

		}

	}
	
	Lo = allPoLight + allDiLight;


	// Add environment diffuse 
	float3 F = fresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);
	float3 kS = F;
	float3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;
	float3 irradiance = irradianceMap.SampleLevel(samp, N, 0).rgb;
	float3 diffuse = irradiance * albedo;

	float3 R = reflect(-V, N);
	const float MAX_REFLECTION_LOD = 4.0;
	float3 prefilteredColor = prefilterMap.SampleLevel(samp, R, roughness * MAX_REFLECTION_LOD).rgb;

	float2 envBRDF = brdfLUT.SampleLevel(samplerForLUT, float2(max(dot(N, V), 0), roughness), 0).rg;
	float3 specular = prefilteredColor * (F*envBRDF.x + envBRDF.y);


	float3 ambient = (kD * diffuse + specular) * ao;

	float3 color = float3(0, 0, 0);


	float width, height;
	shadowMap.GetDimensions(width, height);
	float2 textureSize = float2(width, height);
	float2 texelSize = 1 / textureSize;
	float bias = 0.001f;         //  Set the bias value for fixing the floating point precision issues.
	float3 projCoords = input.posForShadow.xyz / input.posForShadow.w;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;
	projCoords.y = 1 - projCoords.y;


	if ((saturate(projCoords.x) == projCoords.x) && (saturate(projCoords.y) == projCoords.y)) {

		float currentDepth = projCoords.z;
		currentDepth -= bias;
		float shadow = 0;
		for (int i = -3; i < 4; i++) {
			for (int j = -3; j < 4; j++) {
				float cloestDepth = shadowMap.Sample(shadowSampler, projCoords.xy + float2(i, j) * texelSize).r;
				shadow += (currentDepth > cloestDepth ? 1 : 0);
			}
		}
		shadow /= 25;
		color = (ambient + Lo) * (1 - shadow);
	}
	else {
		color = ambient + Lo;
	}
	//color = ambient + Lo;
	// HDR
	color = color / (color + float3(1, 1, 1));
	// Gamma correction
	color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));


	return float4(color, 1);
}
