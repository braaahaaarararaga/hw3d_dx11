﻿#pragma once
#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Material.h"
#include "Mesh.h"

using namespace DirectX;

enum class TextureStorageType
{
	Invalid,
	None,
	EmbeddedIndexCompressed,
	EmbeddedIndexNonCompressed,
	EmbeddedCompressed,
	EmbeddedNonCompressed,
	Disk
};

class Model
{
public:
	bool Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext,
		 ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader,ConstantBuffer<CB_PS_material>& cb_ps_material, IVertexShader * pVertexShader);
	void Draw(const XMMATRIX & world, const XMMATRIX & viewProjectionMatrix);



	
private:
	std::vector<Mesh> meshes;
	bool LoadModel(const std::string& filePath);
	void ProcessNode(aiNode* node,const aiScene* scene, const XMMATRIX& parentTransformMatirx);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatirx);
	TextureStorageType DetermineTextureStorageType(const aiScene* scene, aiMaterial* pMat, unsigned int index, aiTextureType textureType);
	void LoadMaterialTextures(Material& material, aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene);
	int GetTextureIndex(aiString* pStr);

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader = nullptr;
	ConstantBuffer<CB_PS_material>* cb_ps_material = nullptr;
	IVertexShader * pVertexShader = nullptr;
	std::string directory = "";


};