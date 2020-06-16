#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)    
{ 
	float4 colorTint;
	matrix world;
	matrix view;
	matrix proj;
}

V2P_NormalMap main( VertexShaderInput input )
{
	V2P_NormalMap output;

	matrix wvp = mul(proj, mul(view, world));
	output.position = mul(wvp, float4(input.position, 1.0f));

	// @todo: what if the model has none-uniform scales? Make sure to apply the inverse transpose instead of just casting to 3x3
	output.normal = mul((float3x3)world, input.normal);
	// this is new
	output.tangent = mul((float3x3)world, input.tangent);
	output.color = colorTint;
	output.worldPos = mul(world, float4(input.position, 1.0f)).xyz;
	output.uv = input.uv;

	return output;
}