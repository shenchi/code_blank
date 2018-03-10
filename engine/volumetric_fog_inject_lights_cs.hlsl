cbuffer FrameConstants : register (b0)
{
	float4x4				matView;
	float4x4				matProj;
	float4x4				matViewInv;
	float4x4				matProjInv;
	float4					cameraPos;
	float4					bufferSize;
	float4					leftTopRay;
	float4					rightTopRay;
	float4					leftBottomRay;
	float4					rightBottomRay;
	float4					perspectiveParams; //(fov, aspect, zNear, zFar)
	float4					padding3[9];
};

RWTexture3D<float4> lightDensityVolume : register (u0);

float3 SpotLights(float3 pos)
{
	return 0;
}

[numthreads(16, 9, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float2 uv = DTid.xy / float2(159.0, 89.0);
	float z = DTid.z / 127.0;

	float zNear = perspectiveParams.z;
	float zFar = perspectiveParams.w;

	// linear z to cam pos
	z = (zNear + z * (zFar - zNear)) / zFar;

	float3 rayTop = lerp(leftTopRay, rightTopRay, uv.x);
	float3 rayBottom = lerp(leftBottomRay, rightBottomRay, uv.x);

	float3 pos = cameraPos + lerp(rayTop, rayBottom, uv.y) * z;

	float3 color = 0;
	float density = 0;

	color += SpotLights(pos);



	lightDensityVolume[DTid] = float4(color, density);
}