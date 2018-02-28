cbuffer LightParameters : register (b0)
{
	float4x4				transform;
	float4					color;
	float					range;
	float					intensity;
	float					spotAngle;
	float					padding[10 * 4 + 1];
};

float4 main() : SV_TARGET
{
	return color;
}