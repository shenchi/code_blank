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

cbuffer ShadowMatrices : register (b1)
{
	float4x4	matView;
	float4x4	matProj;
	float4x4	padding1;
	float4x4	padding2;
};


struct Input
{
	float3 position	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;
};

float4 main(Input input, uint iid : SV_INSTANCEID) : SV_POSITION
{
	matrix matWorld = matrices[iid].matWorld;

	matrix matMVP = mul(mul(matWorld, matView), matProj);
	return mul(float4(input.position, 1), matMVP);
}
