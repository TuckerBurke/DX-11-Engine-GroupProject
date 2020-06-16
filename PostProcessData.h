#pragma once

struct PostProcessGenericData
{
	float width;
	float height;
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