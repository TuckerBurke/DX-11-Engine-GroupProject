#include "Entity.h"
#include <d3d11.h>
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"
#include "Vertex.h"
#include "SimpleShader.h"

Entity::Entity(Mesh* incomingMesh, Material* incomingMaterial)
{
	mesh = incomingMesh;
	material = incomingMaterial;
	transform = new Transform();
}

Entity::~Entity()
{
	delete transform;
}

Mesh* Entity::GetMesh() const
{
	return mesh;
}

Transform* Entity::GetTransform()
{
	return transform;
}

class Material* Entity::GetMaterial() const
{
	return material;
}

// doesn't involve instanced rendering yet
void Entity::Draw(ID3D11DeviceContext* context, Camera* mainCamera)
{

	SimpleVertexShader* vs = material->GetVertexShader();
	SimplePixelShader* ps = material->GetPixelShader();

	// set the vertex shader data
	vs->SetFloat4("colorTint",  material->GetColorTint());
	vs->SetMatrix4x4("world", transform->GetWorldMatrix());
	vs->SetMatrix4x4("view", mainCamera->GetViewMatrix());
	vs->SetMatrix4x4("proj", mainCamera->GetProjectionMatrix());
	vs->CopyAllBufferData();

	ps->SetFloat("shininess", material->GetShininess());
	ps->CopyAllBufferData();

	ps->SetShaderResourceView("diffuseTexture", material->GetDiffuseTextureWrapper());
	if(material->IsNormalMapMaterial()) 
	{
		ps->SetShaderResourceView("normalMap", material->GetNormalMapWrapper());
	}
	ps->SetSamplerState("samplerOptions", material->GetTextureSampler());

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// set vertex and index buffers and draw the mesh
	context->IASetVertexBuffers(0, 1, mesh->GetVertexBuffer(), &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	context->DrawIndexed
	(
		mesh->GetIndexCount(),
		0,
		0
	);
}

void Entity::DrawTransparent(struct ID3D11DeviceContext* context, class Camera* mainCamera)
{
	SimpleVertexShader* vs = material->GetVertexShader();
	SimplePixelShader* ps = material->GetPixelShader();

	// set the vertex shader data
	vs->SetMatrix4x4("world", transform->GetWorldMatrix());
	vs->SetMatrix4x4("view", mainCamera->GetViewMatrix());
	vs->SetMatrix4x4("proj", mainCamera->GetProjectionMatrix());
	vs->CopyAllBufferData();

	ps->SetFloat4("colorAndAlpha",  material->GetColorTint());
	ps->CopyAllBufferData();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// set vertex and index buffers and draw the mesh
	context->IASetVertexBuffers(0, 1, mesh->GetVertexBuffer(), &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	context->DrawIndexed
	(
		mesh->GetIndexCount(),
		0,
		0
	);
}
