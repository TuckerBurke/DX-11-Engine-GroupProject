#pragma once

#include <DirectXMath.h>

struct Light
{
	DirectX::XMFLOAT3 color;
	float intensity;
	DirectX::XMFLOAT3 direction;
	float range;
	DirectX::XMFLOAT3 position;
	int type;
	float spotFalloff;
	DirectX::XMFLOAT3 pad;
};