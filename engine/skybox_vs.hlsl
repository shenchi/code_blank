cbuffer FrameConstants : register (b0)
{
	float4x4		matView;
	float4x4		matProj;
	float4x4		matViewInv;
	float4x4		matProjInv;
	float4			cameraPos;
	float4			bufferSize;
	float4			leftTopRay;
	float4			rightTopRay;
	float4			leftBottomRay;
	float4			rightBottomRay;
	float4			perspectiveParams;
	float			padding3[4 * 9];
};

struct Input
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

struct V2F
{
	float4 position	: SV_POSITION;
	float3 uv		: TEXCOORD0;
};

V2F main(Input input)
{
	V2F output;

	matrix matViewNoMove = matView;
	matViewNoMove._41 = 0;
	matViewNoMove._42 = 0;
	matViewNoMove._43 = 0;
	matrix matVP = mul(matViewNoMove, matProj);

	output.position = mul(float4(input.position, 1), matVP).xyww;
	output.uv = input.position;

	return output;
}
