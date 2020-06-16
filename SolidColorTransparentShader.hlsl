cbuffer externalData : register(b0)
{
	float4 colorAndAlpha;
}

float4 main() : SV_TARGET
{
	return float4(colorAndAlpha.rgb, clamp(colorAndAlpha.a, 0.f, 1.f));
}