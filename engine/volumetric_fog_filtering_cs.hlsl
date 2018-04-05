
Texture3D<float4> lightDensityVolume : register (t0);
RWTexture3D<float4> filteredLightDensityVolume : register (u0);

#define SAMPLE_RADIUS 2

[numthreads(16, 9, 4)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float4 value = 0;
	float denom = 0;
	for (int z = -SAMPLE_RADIUS; z <= SAMPLE_RADIUS; z++)
	{
		for (int y = -SAMPLE_RADIUS; y <= SAMPLE_RADIUS; y++)
		{
			for (int x = -SAMPLE_RADIUS; x <= SAMPLE_RADIUS; x++)
			{
				int3 pos = DTid + int3(x, y, z);
				pos = max(int3(0, 0, 0), pos);
				pos = min(int3(159, 89, 127), pos);

				float k = (SAMPLE_RADIUS - abs(x)) + (SAMPLE_RADIUS - abs(y)) + (SAMPLE_RADIUS - abs(z)) + 1;

				value += (k * lightDensityVolume[pos]);// / 125.0);
				denom += k;
			}
		}
	}
	filteredLightDensityVolume[DTid] = value / denom;
}
