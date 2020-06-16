#include "Material.h"
#include "SimpleShader.h"
#include "WICTextureLoader.h"

Material::Material(DirectX::XMFLOAT4 colorTint, float shininess, class SimpleVertexShader* VS, class SimplePixelShader* PS)
{
	this->colorTint = colorTint;
	this->shininess = shininess;
	this->vertShader = VS;
	this->pixelShader = PS;
}

Material::Material(DirectX::XMFLOAT4 colorTint, float shininess, ID3D11ShaderResourceView* texResource, ID3D11SamplerState* sampler, SimpleVertexShader* VS, SimplePixelShader* PS)
{
	this->colorTint = colorTint;
	this->shininess = shininess;
	this->diffuseTextureWrapper = texResource;
	textureSampler = sampler;
	this->vertShader = VS;
	this->pixelShader = PS;
}

Material::Material(DirectX::XMFLOAT4 colorTint, float shininess, struct ID3D11ShaderResourceView* diffuseTexture, struct ID3D11ShaderResourceView* normalMapTexture, struct ID3D11SamplerState* sampler, class SimpleVertexShader* VS, class SimplePixelShader* PS)
{
	this->colorTint = colorTint;
	this->shininess = shininess;
	this->diffuseTextureWrapper = diffuseTexture;
	textureSampler = sampler;
	this->vertShader = VS;
	this->pixelShader = PS;
	this->normalMapWrapper = normalMapTexture;
}
