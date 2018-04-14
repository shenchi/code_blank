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

#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_QUALITY__PRESET 12

#define FXAA_GREEN_AS_LUMA 1

#include "FXAA3_11.h"

//float3 fetch(float2 pos)
//{
//	pos = max(pos, float2(0, 0));
//	pos = min(pos, bufferSize.xy);
//
//	return tex.Load(int3(pos, 0)).rgb;
//}
//
//float luma(float3 color)
//{
//	return color.g * (0.587 / 0.299) + color.r;
//}

float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
	//float2 uv = pos.xy - 0.5;

	//float3 rgbN = fetch(uv + float2(0, -1));
	//float3 rgbW = fetch(uv + float2(-1, 0));
	//float3 rgbM = fetch(uv);
	//float3 rgbE = fetch(uv + float2(1, 0));
	//float3 rgbS = fetch(uv + float2(0, 1));

	//float lumaN = luma(rgbN);
	//float lumaW = luma(rgbW);
	//float lumaM = luma(rgbM);
	//float lumaE = luma(rgbE);
	//float lumaS = luma(rgbS);

	//float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaE, lumaS)));
	//float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaE, lumaS)));

	//float range = rangeMax - rangeMin;

	//if (range < max(1.0 / 16.0, rangeMax * 0.125)) 
	//	return float4(rgbM, 1.0f);
	//
	//float lumaLow = (lumaN + lumaW + lumaE + lumaS) * 0.25;
	//float rangeLow = abs(lumaM - lumaLow);
	//float blendLow = max(0.0, (rangeLow / range) - 0.25) * 1.0;
	//blendLow = min(blendLow, 0.75);

	//float3 rgbNW = fetch(uv + float2(-1, -1));
	//float3 rgbNE = fetch(uv + float2(1, -1));
	//float3 rgbSW = fetch(uv + float2(-1, 1));
	//float3 rgbSE = fetch(uv + float2(1, 1));

	//float lumaNW = luma(rgbNW);
	//float lumaNE = luma(rgbNE);
	//float lumaSW = luma(rgbSW);
	//float lumaSE = luma(rgbSE);

	//float3 rgbLow = rgbN + rgbW + rgbM + rgbE + rgbS +
	//	rgbNW + rgbNE + rgbSW + rgbSE;

	//rgbLow *= (1.0 / 9.0);

	//float edgeVert =
	//	abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
	//	abs((0.50 * lumaW) + (-1.0 * lumaM) + (0.50 * lumaE)) +
	//	abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));

	//float edgeHorz =
	//	abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
	//	abs((0.50 * lumaN) + (-1.0 * lumaM) + (0.50 * lumaS)) +
	//	abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));

	//bool horzSpan = (edgeHorz >= edgeVert);

	//return float4(1, 1, 1, 1);

	float2 rcpFrame = 1.0 / bufferSize.xy;

	FxaaTex t = { samp, tex };

	return FxaaPixelShader(
		pos.xy / bufferSize.xy,
		float4(0, 0, 0, 0),
		t,
		t,
		t,
		rcpFrame,
		float4(0, 0, 0, 0),
		float4(0, 0, 0, 0),
		float4(0, 0, 0, 0),
		0.75,
		0.166,
		0.0833,
		8.0,
		0.125,
		0.05,
		float4(1.0, -1.0, 0.25, -0.25)
	);
}
