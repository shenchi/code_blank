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
	float					padding3[4 * 9];
};

Texture2D tex : register (t0);
Texture2D gBuffer2 : register (t1);
Texture3D scatterVolume : register (t2);

SamplerState samp : register (s0);

float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
	float2 screenPos = pos.xy - 0.5;
	float2 uv = screenPos / bufferSize.xy;

	float depth = gBuffer2.Load(int3(screenPos, 0)).a;

	float n = perspectiveParams.z;
	float f = perspectiveParams.w;
	float z = n * depth / (f - depth * (f - n));

	float4 color = tex.Load(int3(screenPos, 0));

	if (z > 0)
	{
		z = min(z, maxZ01 - 0.001) / maxZ01;
		float4 fog = scatterVolume.Sample(samp, float3(uv, z));
		color = color * fog.a + fog;
	}

	return color;
}
