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
	float4					perspectiveParams;
	float					padding3[4 * 9];
};

struct VSInput
{
	float3 position : POSITION;
	float4 color	: COLOR;
	float2 texcoord : TEXCOORD0;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD0;
};

VSOutput main(VSInput input)
{
	VSOutput output;

	float3 pos = input.position;
	pos.xy /= (bufferSize.xy * 0.5);
	pos.y = - pos.y;

	output.position = float4(pos, 1);
	output.color = input.color;
	output.texcoord = input.texcoord;

	return output;
}
