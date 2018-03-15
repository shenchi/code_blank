cbuffer VolumetricFogParams : register (b0)
{
	float4					fogParams;
	float3					windDir;
	float					time;
	float					noiseScale;
	float					noiseBase;
	float					noiseAmp;
	float					density;
	float					maxFarClip;
	float					maxZ01;
}

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

cbuffer SpotLightTransforms : register (b2)
{
	float4x4				spotLightTransfomrs[1024];
};

cbuffer PointLightTransforms : register (b3)
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

cbuffer SpotLightParams : register (b4)
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

cbuffer PointLightParams : register (b5)
{
	PointLight				pointLights[1024];
	uint					numPointLight;
};

RWTexture3D<float4> lightDensityVolume : register (u0);

float hash(float n) { return frac(sin(n) * 753.5453123); }
float noisep(float3 x)
{
	float3 p = floor(x);
	float3 f = frac(x);
	f = f*f*(3.0 - 2.0*f);

	float n = p.x + p.y*157.0 + 113.0*p.z;
	return lerp(lerp(lerp(hash(n + 0.0), hash(n + 1.0), f.x),
		lerp(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
		lerp(lerp(hash(n + 113.0), hash(n + 114.0), f.x),
			lerp(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
}

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
		atten *= pow(max(0, 1 - (1 - DdotL) / r), 0.5);
		
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

		color += atten * pointLights[i].color.rgb * pointLights[i].intensity;
	}

	return color;
}

float Density(float3 pos)
{
	float fog = fogParams.x;

	fog += max(exp(fogParams.y * (-fogParams.y + fogParams.z)) * fogParams.w, 0);

	float3 scroll = windDir * time;
	float3 q = (pos - scroll) * noiseScale;

	float f = 0.75 * noisep(q);

	q = (q + scroll * noiseScale) * 2.01;

	f += 0.25 * noisep(q);
	f = noiseBase + f * noiseAmp;

	return max(fog * f * density, 0);
}

[numthreads(16, 9, 4)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float2 uv = DTid.xy / float2(159.0, 89.0);
	float z = DTid.z / 127.0;

	float zNear = perspectiveParams.z;
	float zActualFar = perspectiveParams.w;
	float zFar = min(maxFarClip, zActualFar);

	// linear z to cam pos
	z = (zNear + z * (zFar - zNear)) / zActualFar;

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
