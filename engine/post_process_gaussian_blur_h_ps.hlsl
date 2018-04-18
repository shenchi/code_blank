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
	float					padding3[4 * 9];
};

Texture2D tex : register(t0);
//SamplerState samp : register(s0);

static float weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

#define RADIUS 5

float3 FetchTexel(float2 uv)
{
	uv = max(0, uv);
	uv = min(bufferSize.xy - 1, uv);
	return tex.Load(int3(uv, 0)).rgb;
}

float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
	//float2 rcpSize = 1.0 / bufferSize.xy;
	//
	//float2 uv = pos.xy * rcpSize;
	//
	//float3 color = tex.Sample(samp, uv).rgb * weights[0];
	//
	//for (int i = 1; i < RADIUS; i++)
	//{
	//	color += tex.Sample(samp, uv + float2(i * rcpSize.x, 0)).rgb * weights[i];
	//	color += tex.Sample(samp, uv - float2(i * rcpSize.x, 0)).rgb * weights[i];
	//}

	float2 uv = pos.xy - 0.5;

	float3 color = tex.Load(int3(uv, 0)).rgb * weights[0];

	for (int i = 1; i < RADIUS; i++)
	{
		color += FetchTexel(uv + float2(i, 0)) * weights[i];
		color += FetchTexel(uv - float2(i, 0)) * weights[i];
	}

	return float4(color, 1.0f);
}