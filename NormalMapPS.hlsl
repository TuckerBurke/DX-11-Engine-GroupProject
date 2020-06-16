#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0) 
{
	Light lights[MAX_LIGHTS];
	int lightCount;

	float3 cameraPosition;
	float shininess;
}

Texture2D diffuseTexture:		register(t0);
Texture2D normalMap:			register(t1);

SamplerState samplerOptions:	register(s0);

float4 main( V2P_NormalMap input ) : SV_TARGET
{
	float3 unpackedNormal = normalMap.Sample(samplerOptions, input.uv).rgb * 2 - 1;
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	float3 N = input.normal;  // Must be normalized
	float3 T = input.tangent; // Must be normalized
	T = normalize(T - N * dot(T, N)); // Gram-Schmidt orthogonalization
	float3 B = cross(T, N); // bi - tangent 
	float3x3 TBN = float3x3(T, B, N);

	// order of the multiplication matters
	input.normal = mul(unpackedNormal, TBN); 

	//return float4(input.normal, 1.f); TESTING
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