cbuffer LightTransforms : register (b0)
{
	float4x4				transforms[1024];
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
	float4					perspectiveParams;
	float					padding3[4 * 9];
};

struct Input
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

struct Output
{
	float4 position : SV_POSITION;
	uint   instanceId : SV_InstanceID;
};

Output main(Input input, uint iid : SV_InstanceID)
{
	Output output;
	matrix matMVP = mul(mul(transforms[iid], matView), matProj);

	output.position = mul(float4(input.position, 1.0f), matMVP);
	output.instanceId = iid;

	//output.position.z = max(output.position.z, 0.001);

	return output;
}