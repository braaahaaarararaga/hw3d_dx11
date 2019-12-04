#include "Model.h"

bool Model::Initialize(const std::string& filePath, ID3D11Device * device, ID3D11DeviceContext * deviceContext,  ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->cb_vs_vertexshader = &cb_vs_vertexshader;
	try 
	{
		if (!this->LoadModel(filePath))
			return false;
	}
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}

	return true;
}

void Model::Draw(const XMMATRIX & world, const XMMATRIX & viewProjectionMatrix)
{
	this->deviceContext->VSSetConstantBuffers(0, 1, this->cb_vs_vertexshader->GetAddressOf());
	
	
	for (int i = 0; i < meshes.size(); i++)
	{
		this->cb_vs_vertexshader->data.wvpMatrix = meshes[i].GetTransformMatrix() * world * viewProjectionMatrix; // Calculate world-view-projection matrix
		this->cb_vs_vertexshader->data.worldMatrix = meshes[i].GetTransformMatrix() * world;
		this->cb_vs_vertexshader->ApplyChanges();
		meshes[i].Draw();
	}
}

bool Model::LoadModel(const std::string & filePath)
{
	this->directory = StringHelper::GetDirectoryFromPath(filePath);
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filePath,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded);

	if (pScene == NULL)
		return false;

	this->ProcessNode(pScene->mRootNode, pScene, DirectX::XMMatrixIdentity());
	return true;
}

void Model::ProcessNode(aiNode * node,const aiScene * scene, const XMMATRIX& parentTransformMatirx)
{
	XMMATRIX nodeTransformMatrix = XMMatrixTranspose(XMMATRIX(&node->mTransformation.a1)) * parentTransformMatirx;
	

	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(this->ProcessMesh(mesh, scene, nodeTransformMatrix));
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		this->ProcessNode(node->mChildren[i], scene, nodeTransformMatrix);
	}
}

Mesh Model::ProcessMesh(aiMesh * mesh, const aiScene * scene, const XMMATRIX& transformMatirx)
{
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;
	Material material;

	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.pos.x = mesh->mVertices[i].x;
		vertex.pos.y = mesh->mVertices[i].y;
		vertex.pos.z = mesh->mVertices[i].z;

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		if (mesh->mTextureCoords[0])
		{
			vertex.texCoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texCoord.y = (float)mesh->mTextureCoords[0][i].y;
		}
		vertices.push_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	aiMaterial* pMaterial = scene->mMaterials[mesh->mMaterialIndex];
	LoadMaterialTextures(material, pMaterial, aiTextureType::aiTextureType_DIFFUSE, scene);


	return Mesh(this->device, this->deviceContext, vertices, indices, material, transformMatirx);
}

TextureStorageType Model::DetermineTextureStorageType(const aiScene * scene, aiMaterial * pMat, unsigned int index, aiTextureType textureType)
{
	if (pMat->GetTextureCount(textureType) == 0)
		return TextureStorageType::None;

	aiString path;
	pMat->GetTexture(textureType, index, &path);
	std::string texturePath = path.C_Str();
	// check if texture is an embedded indexed texture by seeing if the file path is an index #
	if (texturePath[0] == '*')
	{
		if (scene->mTextures[0]->mHeight == 0)
		{
			return TextureStorageType::EmbeddedIndexCompressed;
		}
		else
		{
			assert("SUPPORT DOES NOT EXIST YET FOR INDEXED NON COMPRESSED TEXTURES!" && 0);
			return TextureStorageType::EmbeddedIndexNonCompressed;
		}
	}
	// check if texture is an embedded texture but no indexed (path will be the texture's name insead of #)
	if (auto pTex = scene->GetEmbeddedTexture(texturePath.c_str()))
	{
		if (pTex->mHeight == 0)
		{
			return TextureStorageType::EmbeddedCompressed;
		}
		else
		{
			assert("SUPPORT DOES NOT EXIST YET FOR INDEXED NON COMPRESSED TEXTURES!" && 0);
			return TextureStorageType::EmbeddedIndexNonCompressed;
		}
		// lastly check if texture is a filepath by checking for period before extension name
	}

	if (texturePath.find('.') != std::string::npos)
	{
		return TextureStorageType::Disk;
	}
	return TextureStorageType::None; // No Texture exists
}

void Model::LoadMaterialTextures(Material& material, aiMaterial * pMaterial, aiTextureType textureType, const aiScene * pScene)
{
	Resource<Texture> pTexture = nullptr;
	TextureStorageType storetype = TextureStorageType::Invalid;
	unsigned int textureCount = pMaterial->GetTextureCount(textureType);

	if (textureCount == 0)
	{
		storetype = TextureStorageType::None;
		aiColor3D aiColor(0.0f, 0.0f, 0.0f);
		switch (textureType)
		{
		case aiTextureType_DIFFUSE:
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
			if (aiColor.IsBlack())
			{
				pTexture = ResourceManager::GetTexture(this->device, std::string("UnloadedTextureColor"), Hw3d_Colors::UnloadedTextureColor, textureType);
			}
			pTexture = ResourceManager::GetTexture(this->device, std::string("BlackTexture"), Color(aiColor.r * 255, aiColor.g * 255, aiColor.b * 255),
				textureType);
			break;
		}
	}
	else
	{
		for (UINT i = 0; i < textureCount; i++)
		{
			aiString path;
			pMaterial->GetTexture(textureType, i, &path);
			TextureStorageType storetype = DetermineTextureStorageType(pScene, pMaterial, i, textureType);
			switch (storetype)
			{
			case TextureStorageType::EmbeddedIndexCompressed:
			{
				int index = GetTextureIndex(&path);
				pTexture = std::make_shared<Texture>(this->device,
					reinterpret_cast<char*>(pScene->mTextures[index]->pcData),
					pScene->mTextures[index]->mWidth,
					textureType);
				break;
			}
			case TextureStorageType::EmbeddedIndexNonCompressed:
			{
				int index = GetTextureIndex(&path);
				pTexture = std::make_shared<Texture>(this->device,
					reinterpret_cast<char*>(pScene->mTextures[index]->pcData),
					pScene->mTextures[index]->mWidth * pScene->mTextures[index]->mHeight,
					textureType);
				break;
			}
			case TextureStorageType::EmbeddedCompressed:
			{
				const aiTexture* pAiTex = pScene->GetEmbeddedTexture(path.C_Str());
				pTexture = std::make_shared<Texture>(this->device,
					reinterpret_cast<char*>(pAiTex->pcData),
					pAiTex->mWidth,
					textureType);
				break; 
			}
			case TextureStorageType::EmbeddedNonCompressed:
			{
				const aiTexture* pAiTex = pScene->GetEmbeddedTexture(path.C_Str());
				pTexture = std::make_shared<Texture>(this->device,
					reinterpret_cast<char*>(pAiTex->pcData),
					pAiTex->mWidth * pAiTex->mHeight,
					textureType);
				break;
			}
			case TextureStorageType::Disk:
			{
				std::string filename = this->directory + '\\' + path.C_Str();
				pTexture = ResourceManager::GetTexture(this->device, filename, textureType);
				break;
			}
			}

		}
	}


	switch (textureType)
	{
	case aiTextureType_AMBIENT:
		material.SetAmbientTexture(std::move(pTexture));
		break;
	case aiTextureType_DIFFUSE:
		material.SetDiffuseTexture(std::move(pTexture));
		break;
	case aiTextureType_SPECULAR:
		material.SetSpecularTexture(std::move(pTexture));
		break;
	case aiTextureType_EMISSIVE:
		material.SetEmissiveTexture(std::move(pTexture));
		break;
	case aiTextureType_NORMALS:
	case aiTextureType_HEIGHT:
		material.SetNormalTexture(std::move(pTexture));
		break;
	default:
		assert(false, "Unknown texture type");
	}
}

int Model::GetTextureIndex(aiString * pStr)
{
	assert(pStr->length >= 2);

	return atoi(&pStr->C_Str()[1]);
}
