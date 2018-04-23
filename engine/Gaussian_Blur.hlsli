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
SamplerState samp : register(s0);

static float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
static float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

struct Input
{
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD0;
};

float4 main(Input input) : SV_TARGET
{
	float2 uv = input.uv;

	float2 texelSize = 1.0f / bufferSize.xy;
#ifdef TOFU_BLUR_HORIZONTAL
	float2 step = float2(texelSize.x, 0);
#else
	float2 step = float2(0, texelSize.y);
#endif

	float3 color = tex.Sample(samp, uv).rgb * weight[0];

	color += tex.Sample(samp, uv + step * offset[1]).rgb * weight[1];
	color += tex.Sample(samp, uv - step * offset[1]).rgb * weight[1];

	color += tex.Sample(samp, uv + step * offset[2]).rgb * weight[2];
	color += tex.Sample(samp, uv - step * offset[2]).rgb * weight[2];

	return float4(color, 1.0f);
}
