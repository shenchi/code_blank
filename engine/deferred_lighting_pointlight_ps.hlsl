cbuffer LightParameters : register (b0)
{
	float4x4				transform;
	float4x4				matLightView;
	float4x4				matLightProj;
	float4					direction;
	float4					color;
	float					range;
	float					intensity;
	float					spotAngle;
	float					padding[1 * 4 + 1];
};

cbuffer FrameConstants : register (b1)
{
	matrix	matView;
	matrix	matProj;
	matrix	matViewInv;
	matrix	matProjInv;
	float4	cameraPos;
	float4	bufferSize;
};

Texture2D gBuffer1 : register(t0);
Texture2D gBuffer2 : register(t1);
Texture2D gBuffer3 : register(t2);

SamplerState samp : register(s0);

float4 main(float4 clipPos : SV_POSITION) : SV_TARGET
{
	float3 lightPos = transform[3].rgb;
	float2 screenPos = clipPos.xy - 0.5;
	
	float4 texel = gBuffer2.Load(int3(screenPos, 0)).rgba;
	float4 worldPos = float4(screenPos, texel.a, 1);
	worldPos.xy = (worldPos.xy / bufferSize.xy) * 2 - 1;
	worldPos.y = -worldPos.y;
	worldPos = mul(worldPos, matProjInv);
	worldPos /= worldPos.w;
	worldPos = mul(worldPos, matViewInv);
	
	float3 worldNormal = texel.rgb;
	float3 albedo = gBuffer3.Load(int3(screenPos, 0)).rgb;

	float3 lightDir = lightPos - worldPos,xyz;
	float dist = length(lightDir);
	lightDir = normalize(lightDir);

	float NdotL = max(dot(lightDir, worldNormal), 0);
	float atten = max(0, 1 - (dist / range));

	float value = NdotL * atten;

	return float4(value * color.rgb * albedo, 1);
}