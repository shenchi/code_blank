Texture3D<float4> lightDensityVolume : register(t0);
RWTexture3D<float4> scatterVolume : register(u0);

[numthreads(16, 9, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint3 pos = uint3(DTid.xy, 0);
	float4 v = float4(0, 0, 0, 1);

	for (uint z = 0; z < 128; z++)
	{
		pos.z = z;
		
		float4 lightDensity = lightDensityVolume[pos];

		float density = max(lightDensity.w, 0.000001);
		float transmittance = exp(-density / 128.0);

		float3 integral = lightDensity.xyz  * (1.0 - transmittance) / density;

		v.xyz += integral * v.w;
		v.w *= transmittance;

		scatterVolume[pos] = v;
		//scatterVolume[pos] = lightDensity;
	}
}
