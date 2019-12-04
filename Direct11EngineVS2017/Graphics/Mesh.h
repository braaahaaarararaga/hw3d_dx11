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
	std::vector<DirectX::XMFLOAT2> texcoord;
	std::vector<DirectX::XMFLOAT3> tangent;
	std::vector<DirectX::XMFLOAT3> bitangent;
	std::vector<DirectX::XMFLOAT4> bone_names;
	std::vector<DirectX::XMFLOAT4> bone_weights;
};

class Mesh
{
public:
	Mesh(ID3D11Device * device, ID3D11DeviceContext* deviceContext, MeshParameters& params, std::vector<DWORD>& indices, Material& material, const DirectX::XMMATRIX& matrixTransform);
	Mesh(const Mesh& mesh);

	void Bind(IVertexShader* pVertexShader) const;

	void SetData(MeshParameters& params);

	void Draw(IVertexShader* pVertexShader);
	const DirectX::XMMATRIX& GetTransformMatrix();
private:
	VertexBuffer<DirectX::XMFLOAT3> vBufPosition;
	VertexBuffer<DirectX::XMFLOAT4> vBufColor;
	VertexBuffer<DirectX::XMFLOAT3> vBufNormal;
	VertexBuffer<DirectX::XMFLOAT2> vBufTexCoord;
	VertexBuffer<DirectX::XMFLOAT3> vBufTangent;
	VertexBuffer<DirectX::XMFLOAT3> vBufBitangent;
	VertexBuffer<DirectX::XMFLOAT4> vBufBoneNames;
	VertexBuffer<DirectX::XMFLOAT4> vBufBoneWeights;
	IndexBuffer indexBuffer;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	Material material;
	DirectX::XMMATRIX transformMatrix;
};

