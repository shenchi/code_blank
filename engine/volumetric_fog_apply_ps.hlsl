cbuffer FrameConstants : register (b0)
{
	matrix	matView;
	matrix	matProj;
	matrix	matViewInv;
	matrix	matProjInv;
	float4	cameraPos;
	float4	bufferSize;
};

Texture2D fog : register (t0);

SamplerState samp : register (s0);

float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
	float2 uv = (pos.xy - 0.5) / bufferSize.xy;
	return fog.Sample(samp, uv);
}
