cbuffer LightParameters : register (b0)
{
	float4x4				transform;
	float4					color;
	float					range;
	float					intensity;
	float					spotAngle;
	float					padding[10 * 4 + 1];
};

cbuffer ScreenParameters : register (b1)
{
	float4					bufferSize;
}

Texture2D gBuffer1 : register(t0);
Texture2D gBuffer2 : register(t1);
Texture2D gBuffer3 : register(t2);

SamplerState samp : register(s0);

float4 main(float4 clipPos : SV_POSITION) : SV_TARGET
{
	float3 lightPos = transform[3].rgb;
	float2 screenPos = clipPos.xy / clipPos.w / bufferSize.xy;
	
	return float4(gBuffer3.Load(int3(screenPos, 0)).rgb, 1);
}