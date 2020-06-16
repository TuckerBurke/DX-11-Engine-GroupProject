#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__ 

// this is the maximum a shader can hold for light processing
#define MAX_LIGHTS 128 

#define LIGHT_TYPE_DIR 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define LIGHT_TYPE_AMBIENT 3

struct Light 
{
	float3 color;
	float intensity;

	float3 direction;
	float range;

	float3 position;
	int type;

	float spotFalloff;
	float3 pad;
};

// Doesn't exist inside constant buffers - thus an be unaligned
struct PixelData 
{
	float3 worldPos; // pixel position in world space
	float shininess; // how shiny is the pixel
	float3 normal; // the normal of the pixel
};

struct VertexShaderInput
{ 
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 position		: POSITION;     // XYZ position
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float4 color		: COLOR;        // RGBA color
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION;
};

struct V2P_NormalMap
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float4 color		: COLOR;        // RGBA color
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 worldPos		: POSITION;
	float3 tangent		: TANGENT;
};

// HELPER FUNCTIONS

float3 NormalizedDirToLight(float3 lightDir)
{
	return normalize(-1 * lightDir);
}

float NdotL(float3 normal, float3 nDirToLight)
{
	return saturate(dot(normal, nDirToLight));
}

float Diffuse(float3 normal, float3 lightDir) 
{
	float3 nDirToLight = NormalizedDirToLight(lightDir);
	return NdotL(normal, nDirToLight);
}

float SpecularPhong(float3 normal, float3 lightDir, float3 dirToCamera, float exp) 
{
	// Calculate light reflection vector
	float3 refl = reflect(lightDir, normal);

	return pow(saturate(dot(refl, dirToCamera)), exp);
}

float Attenuate(Light light, float3 worldPos)
{
	float dist = distance(light.position, worldPos);

	// Ranged-based attenuation
	float att = saturate(1.0f - (dist * dist / (light.range * light.range)));

	// Soft falloff
	return att * att;
}

float3 OutputFinalLight(float diffuse, float spec, float materialShininess, Light light) 
{
	spec *= any(diffuse);
	return ( (diffuse + ( materialShininess * spec ) ) * (light.intensity * light.color)); // optimized for superscalar
}

float3 PointLight(PixelData pixelData, float3 cameraPosition, Light light) 
{
	float3 toCamera = normalize(cameraPosition - pixelData.worldPos);
	float3 pointLightDirection = normalize(pixelData.worldPos - light.position);

	float atten = Attenuate(light, pixelData.worldPos);
	float diffuse = Diffuse(pixelData.normal, pointLightDirection);
	float spec = SpecularPhong(pixelData.normal, pointLightDirection, toCamera, 64.0f);

	return OutputFinalLight(diffuse, spec, pixelData.shininess, light) * atten;
}

float3 DirectionLight(PixelData pixelData, float3 cameraPosition, Light light)
{
	float3 toCamera = normalize(cameraPosition - pixelData.worldPos);

	float diffuse = Diffuse(pixelData.normal, light.direction);
	float spec = SpecularPhong(pixelData.normal, normalize(light.direction), toCamera, 64.0f);

	return OutputFinalLight(diffuse, spec, pixelData.shininess, light);
}

float3 AmbientLight(Light light) 
{
	return light.intensity * light.color;
}

float3 SpotLight(PixelData pixelData, float3 cameraPosition, Light light)
{
	// Calculate the spot falloff
	float3 toLight = normalize(light.position - pixelData.worldPos);
	float penumbra = pow(saturate(dot(-toLight, light.direction)), light.spotFalloff);
	
	// Combine with the point light calculation
	return PointLight(pixelData, cameraPosition, light) * penumbra;
}

/*
 * Deprecated Function
 * Use only for testing
*/
float4 CalculateLights(float3 normal, float4 surfaceColor, float3 worldPos, Light lights[64], float3 cameraPosition, float shininess, int lightCount) 
{
	// vector from world space pixel pos to camera world world space position
	float3 toCamera = normalize(cameraPosition - worldPos);
	float3 finalLight = float3(0,0,0);

	for (int i = 0; i < lightCount; i++) 
	{
		// directional diffuse
		float diffuse = 0;

		// specular of the the material (how shiny it is)
		float spec = 0;
		
		// point light diffuse & spec
		switch(lights[i].type) 
		{
		case LIGHT_TYPE_POINT:
			float3 pointLightDirection = normalize(worldPos - lights[i].position);
			diffuse = Diffuse(normal, pointLightDirection);
			spec = SpecularPhong(normal, pointLightDirection, toCamera, 64.0f);
			break;
		case LIGHT_TYPE_DIR:
			diffuse = Diffuse(normal, lights[i].direction);
			spec = SpecularPhong(normal, normalize(lights[i].direction), toCamera, 64.0f);
			break;
		case LIGHT_TYPE_SPOT:
			// @todo
			break;
		case LIGHT_TYPE_AMBIENT:
			diffuse = 1.0f;
			spec = 0;
			break;
		}
		spec *= any(diffuse);
		finalLight += ( (diffuse + ( shininess * spec ) ) * lights[i].intensity * lights[i].color);
	}

	// @todo: think if you actually want to consider the surfaceColor here with the specular
	// @todo: specular lights will overlap each other, consider abstracting each light's specular and multiplying them all together
	return float4(finalLight * (float3)surfaceColor, 1);
}


#endif
