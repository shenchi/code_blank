RWTexture2D<float4> tex : register(u0);

[numthreads(16, 9, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float4 color = float4(DTid.xy / float2(159.0, 89.0), 0, 1);
	tex[DTid.xy] = color;
}
