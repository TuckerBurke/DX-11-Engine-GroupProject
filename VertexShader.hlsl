#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)    
{ 
	float4 colorTint;
	matrix world;
	matrix view;
	matrix proj;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	matrix wvp = mul(proj, mul(view, world));
	output.position = mul(wvp, float4(input.position, 1.0f));

	// @todo: what if the model has none-uniform scales? Make sure to apply the inverse transpose instead of just casting to 3x3
	output.normal = mul((float3x3)world, input.normal);
	output.color = colorTint;
	output.worldPos = mul(world, float4(input.position, 1.0f)).xyz;
	output.uv = input.uv;
	return output;
}