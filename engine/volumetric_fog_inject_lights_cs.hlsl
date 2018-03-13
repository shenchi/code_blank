cbuffer SpotLightTransforms : register (b0)
{
	float4x4				spotLightTransfomrs[1024];
};

cbuffer FrameConstants : register (b1)
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

struct SpotLight
{
	float4					direction;
	float4					color;
	float					intensity;
	float					range;
	float					spotAngle;
	uint					shadowId;
};

cbuffer SpotLightParams : register (b2)
{
	SpotLight				spotLights[1024];
	uint					numSpotLight;
};

//struct LightVP
//{
//	float4x4				matView;
//	float4x4				matProj;
//	float4x4				_padding1;
//	float4x4				_padding2;
//};
//
//cbuffer ShadowTransforms : register (b3)
//{
//	LightVP					matLightVPs[16];
//}


RWTexture3D<float4> lightDensityVolume : register (u0);

float3 SpotLights(float3 pos)
{
	float color = 0;

	for (uint i = 0; i < numSpotLight; i++)
	{
		float3 lightPos = spotLightTransfomrs[i][3].xyz;
		float3 L = lightPos - pos;
		float distNorm = dot(L, L) / (spotLights[i].range * spotLights[i].range);
		float atten = (1 / (distNorm * distNorm)) * (1 - step(1, distNorm));;

		L = normalize(L);
		float DdotL = dot(-spotLights[i].direction.xyz, L);

		float r = 1 - spotLights[i].spotAngle;
		atten *= pow(max(0, 1 - (1 - DdotL) / r), 2.0);
		
		color += atten * spotLights[i].color.rgb * spotLights[i].intensity;
	}

	return color;
}

float Density(float3 pos)
{
	return max(exp(pos.y), 0);
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

	density = Density(pos);

	//lightDensityVolume[DTid] = float4(color * density, density);
	lightDensityVolume[DTid] = float4(pos, density);
}
