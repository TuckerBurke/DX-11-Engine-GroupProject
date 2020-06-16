struct PostProcessGenericData
{
	float pixelWidth;
	float pixelHeight;
};

struct VignetteData 
{
	// 8 bytes
	PostProcessGenericData ppgData;

	// 8 bytes
	float innerRadius;
	float outerRadius;

	// this will be on the next cache line
	float opacity;
};

cbuffer externalData : register(b0)
{
	VignetteData vignetteData;
}


// Defines the input to this pixel shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D pixels			: register(t0);
SamplerState samplerOptions	: register(s0);

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float2 uv = input.uv + float2(vignetteData.ppgData.pixelWidth, vignetteData.ppgData.pixelHeight);
	float4 totalColor = pixels.Sample(samplerOptions, uv);
	float2 centered = uv - float2(0.5f, 0.5f);
    
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Create the vignette effect in black and white
    color.rgb *= 1.0f - smoothstep(vignetteData.innerRadius, vignetteData.outerRadius, length(centered));
    
    // Apply the vignette to the image
    color *= totalColor;
    
    // Lerp between the vignette version and the original to change the vignette opacity
    color = lerp(totalColor, color, vignetteData.opacity);
    

	return (color);
}