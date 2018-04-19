struct InstanceMatrices
{
	matrix	matWorld;
	matrix	matWorld_IT;
	matrix  padding1;
	matrix  padding2;
};

cbuffer InstanceConstants : register (b0)
{
	InstanceMatrices	matrices[256];
};

cbuffer FrameConstants : register (b1)
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
	float3 worldPos	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

V2F main(Input input, uint iid : SV_INSTANCEID)
{
	V2F output;

	matrix matWorld = matrices[iid].matWorld;
	matrix matWorld_IT = matrices[iid].matWorld_IT;

	matrix matMVP = mul(mul(matWorld, matView), matProj);

	output.position = mul(float4(input.position, 1), matMVP);
	output.worldPos = mul(float4(input.position, 1), matWorld).xyz;
	output.normal = normalize(mul(input.normal, (float3x3)matWorld_IT));
	output.tangent = float4(normalize(mul(input.tangent.xyz, (float3x3)matWorld_IT)), input.tangent.w);
	//output.normal = mul(input.normal, (float3x3)matWorld);
	//output.tangent = float4(mul(input.tangent.xyz, (float3x3)matWorld), input.tangent.w);
	output.uv = input.uv;

	return output;
}
