#pragma once

#include <wrl/client.h>

struct Vertex;
struct ID3D11Device;
struct ID3D11Buffer;

class Mesh
{
public:
	Mesh(struct Vertex* vertexData, unsigned int vertexCount, unsigned int* indices, int indexCount, struct ID3D11Device* device);
	Mesh(const char* fileName, struct ID3D11Device* device);
	~Mesh() = default;

	struct ID3D11Buffer* const* GetVertexBuffer() const;
	struct ID3D11Buffer* GetIndexBuffer() const;
	int GetIndexCount() const;

private:

	void GenerateVertAndIndexBuffers(struct Vertex* vertexData, unsigned int vertexCount, unsigned int* indices, int indexCount, struct ID3D11Device* device);
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

	Microsoft::WRL::ComPtr<struct ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<struct ID3D11Buffer> indexBuffer;
	
	int indexBufferCount = 0;
};
