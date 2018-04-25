struct V2F
{
	float4 position	: SV_POSITION;
	float3 worldPos	: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float2 uv		: TEXCOORD0;

};

cbuffer MaterialParams : register (b0)
{
	float4		color;
	float4		emissionColor;
	float4		textureParams;
}

Texture2D diffuseTex : register(t0);
Texture2D emissionMap : register(t1);

SamplerState samp : register(s0);

float4 main(V2F input) : SV_TARGET
{
	float2 uv = (input.uv * textureParams.xy) + textureParams.zw;

	float4 diff = diffuseTex.Sample(samp, uv);
	float4 emm = emissionMap.Sample(samp, uv);
	
	float alpha = max(step(0.01, emm.r + emm.g + emm.b), diff.a);
	
	return float4(diff.rgb * color.rgb + 10.0 * emm.rgb * emissionColor.rgb, alpha);
}
