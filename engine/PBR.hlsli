
static float PI = 3.14159265359f;

float DistributionGGX(float NdotH, float roughness)
{
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH2 = NdotH*NdotH;

	float nom = a2;
	float denom = NdotH2*(a2 - 1.0) + 1.0;
	denom = PI * denom*denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = roughness + 1.0;  //direct lighting
	float k = r*r / 8.0;

	float nom = NdotV;
	float denom = NdotV*(1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
	float ggx1 = GeometrySchlickGGX(NdotV, roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1*ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0) {
	return F0 + (1.0 - F0)*pow(1.0 - cosTheta, 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness) {
	return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 LightingModel(float3 radiance, float3 albedo, float NdotH, float HdotV, float NdotV, float NdotL, float metallic, float roughness)
{
	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, metallic);
	float3 F = FresnelSchlick(HdotV, F0);

	float NDF = DistributionGGX(NdotH, roughness);
	float G = GeometrySmith(NdotV, NdotL, roughness);

	float3 nominator = NDF * F * G;
	float denominator = max(4 * NdotV * NdotL, 0.001); //avoid divide 0
	float3 specular = nominator / denominator;

	float3 kS = F;
	float3 kD = float3(1, 1, 1) - kS;
	kD *= 1 - metallic;

	float3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

	return Lo;
}

float3 EnvironmentLight(float3 N, float3 V, float3 albedo, float ao, float metallic, float roughness, 
	TextureCube irradianceMap, TextureCube prefilterMap, Texture2D brdfLUT, SamplerState samp, SamplerState samplerForLUT) {
	// Add environment diffuse 
	float3 F0 = float3(0.04, 0.04, 0.04);
	float3 F = FresnelSchlickRoughness(max(dot(N, V), 0), F0, roughness);
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
	return ambient;
}