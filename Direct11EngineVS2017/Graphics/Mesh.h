#pragma once
#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "Material.h"
#include "Texture.h"
#include "../ResourceManager.h"

struct MeshParameters
{
	std::vector<DirectX::XMFLOAT3> position;
	std::vector<DirectX::XMFLOAT4> color;
	std::vector<DirectX::XMFLOAT3> normal;
	std::vector<DirectX::XMFLOAT2> uv;
	std::vector<DirectX::XMFLOAT3> tangent;
	std::vector<DirectX::XMFLOAT3> bitangent;
	std::vector<DirectX::XMFLOAT4> bone_ids;
	std::vector<DirectX::XMFLOAT4> bone_weights;
};

class Mesh
{
public:
	Mesh(ID3D11Device * device, ID3D11DeviceContext* deviceContext, std::vector<Vertex>& vertices, std::vector<DWORD>& indices, Material& material, const DirectX::XMMATRIX& matrixTransform);
	Mesh(const Mesh& mesh);
	void Draw();
	const DirectX::XMMATRIX& GetTransformMatrix();
private:
	VertexBuffer<Vertex> vertexbuffer;
	IndexBuffer indexBuffer;
	ID3D11DeviceContext* deviceContext;
	Material material;
	DirectX::XMMATRIX transformMatrix;
};

