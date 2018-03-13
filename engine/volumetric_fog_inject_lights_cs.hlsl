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

cbuffer SpotLightTransforms : register (b1)
{
	float4x4				spotLightTransfomrs[1024];
};

cbuffer PointLightTransforms : register (b2)
{
	float4x4				pointLightTransfomrs[1024];
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

cbuffer SpotLightParams : register (b3)
{
	SpotLight				spotLights[1024];
	uint					numSpotLight;
};

struct PointLight
{
	float4					color;
	float					intensity;
	float					range;
	float					_padding1;
	float					_padding2;
};

cbuffer PointLightParams : register (b4)
{
	PointLight				pointLights[1024];
	uint					numPointLight;
};

//struct LightVP
//{
//	float4x4				matView;
//	float4x4				matProj;
//	float4x4				matVP;
//	float4x4				_padding2;
//};
//
//cbuffer ShadowTransforms : register (b3)
//{
//	LightVP					matLightVPs[16];
//}
//
//Texture2DArray shadowMaps : register(t0);
//SamplerState shadowSamp : register(s0);

RWTexture3D<float4> lightDensityVolume : register (u0);

float3 SpotLights(float3 pos)
{
	float3 color = 0;

	for (uint i = 0; i < numSpotLight; i++)
	{
		float3 lightPos = spotLightTransfomrs[i][3].xyz;
		float3 L = lightPos - pos;
		float distNorm = dot(L, L) / (spotLights[i].range * spotLights[i].range);
		float atten = (1 / (1 + distNorm)) * (1 - step(1, distNorm));;

		L = normalize(L);
		float DdotL = dot(-spotLights[i].direction.xyz, L);

		float r = 1 - spotLights[i].spotAngle;
		atten *= pow(max(0, 1 - (1 - DdotL) / r), 2.0);
		
		/*uint shadowId = spotLights[i].shadowId;
		if (shadowId < 16)
		{
			float4 lightSpacePos = mul(float4(pos, 1), matLightVPs[shadowId].matVP);

			lightSpacePos /= lightSpacePos.w;
			lightSpacePos.xy = lightSpacePos.xy * 0.5 + 0.5;
			lightSpacePos.y = 1 - lightSpacePos.y;

			float currentDepth = lightSpacePos.z;
			currentDepth -= 0.00001;
			float cloestDepth = shadowMaps.SampleLevel(
				shadowSamp, float3(lightSpacePos.xy, shadowId), 0).r;

			float shadow = step(cloestDepth, currentDepth);
			atten *= (1 - shadow);
		}*/

		color += atten * spotLights[i].color.rgb * spotLights[i].intensity;
	}

	return color;
}

float3 PointLights(float3 pos)
{
	float3 color = 0;

	for (uint i = 0; i < numPointLight; i++)
	{
		float3 lightPos = pointLightTransfomrs[i][3].xyz;
		float3 L = lightPos - pos;
		float distNorm = dot(L, L) / (pointLights[i].range * pointLights[i].range);
		float atten = (1 / (1 + distNorm)) * (1 - step(1, distNorm)) * (1 - distNorm);

		//L = normalize(L);
		//float DdotL = dot(-spotLights[i].direction.xyz, L);

		//float r = 1 - spotLights[i].spotAngle;
		//atten *= pow(max(0, 1 - (1 - DdotL) / r), 2.0);

		color += atten * pointLights[i].color.rgb * pointLights[i].intensity;
	}

	return color;
}

float Density(float3 pos)
{
	return 2;
	//return max(exp(pos.y), 0);
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

	float3 rayTop = lerp(leftTopRay.xyz, rightTopRay.xyz, uv.x);
	float3 rayBottom = lerp(leftBottomRay.xyz, rightBottomRay.xyz, uv.x);

	float3 pos = cameraPos.xyz + lerp(rayTop, rayBottom, uv.y) * z;

	float3 color = 0;
	float density = 0;

	color += SpotLights(pos);

	color += PointLights(pos);

	density = Density(pos);

	lightDensityVolume[DTid] = float4(color * density, density);
}
