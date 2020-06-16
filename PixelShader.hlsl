#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0) 
{
	Light lights[MAX_LIGHTS];
	int lightCount;

	float3 cameraPosition;
	float shininess;
}

Texture2D diffuseTexture: register(t0);

SamplerState samplerOptions: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	input.color = diffuseTexture.Sample(samplerOptions, input.uv) * input.color;

	float3 finalLight = float3(0,0,0);

	PixelData pixelData;
	pixelData.normal = input.normal;
	pixelData.worldPos = input.worldPos;
	pixelData.shininess = shininess;

	for (int i = 0; i < lightCount; i++) 
	{
		// point light diffuse & spec
		switch(lights[i].type) 
		{
		case LIGHT_TYPE_POINT:
			///
			finalLight += PointLight(pixelData, cameraPosition, lights[i]);
			///
			break;
		case LIGHT_TYPE_DIR:
			///
			finalLight += DirectionLight(pixelData, cameraPosition, lights[i]);
			///
			break;
		case LIGHT_TYPE_SPOT:
			///
			finalLight += SpotLight(pixelData, cameraPosition, lights[i]);
			///
			break;
		case LIGHT_TYPE_AMBIENT:
			///
			finalLight += AmbientLight(lights[i]);
			///
			break;
		}
	}

	return float4(finalLight * (float3)input.color, 1);
}