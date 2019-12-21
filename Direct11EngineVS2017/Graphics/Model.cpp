#include "Model.h"

static DirectX::XMMATRIX AiToDxMatrix(const aiMatrix4x4& aimat)
{
	return DirectX::XMMatrixTranspose(DirectX::XMMATRIX(&aimat.a1));
}


bool Model::Initialize(const std::string& filePath, ID3D11Device * device, ID3D11DeviceContext * deviceContext,  ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader,
	ConstantBuffer<CB_PS_material>& cb_ps_material, IVertexShader * pVertexShader)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->cb_vs_vertexshader = &cb_vs_vertexshader;
	this->cb_ps_material = &cb_ps_material;
	this->pVertexShader = pVertexShader;
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
		this->cb_vs_vertexshader->data.vpMatrix = viewProjectionMatrix; // Calculate world-view-projection matrix
		this->cb_vs_vertexshader->data.worldMatrix = meshes[i].GetTransformMatrix() * world;
		this->cb_vs_vertexshader->ApplyChanges();
		meshes[i].Draw(pVertexShader);
	}
}

bool Model::InitAnimation(ConstantBuffer<CB_Bones>* cbufBone, MeshAnimator* animator_out, AnimationComponent* animComp)
{
	LoadAnimations(animator_out, cbufBone, animComp);
	return true;
}

bool Model::LoadModel(const std::string & filePath)
{
	this->directory = StringHelper::GetDirectoryFromPath(filePath);

	m_pScene = m_Importer.ReadFile(filePath,
		aiProcess_ConvertToLeftHanded |
		aiProcessPreset_TargetRealtime_Fast |
		aiProcess_TransformUVCoords);

	if (m_pScene == NULL)
		return false;

	meshes.reserve(m_pScene->mNumMeshes);
	for (unsigned int mesh_index = 0; mesh_index < m_pScene->mNumMeshes; mesh_index++)
	{
		aiMesh* pMesh = m_pScene->mMeshes[mesh_index];
		for (unsigned int bone_index = 0; bone_index < pMesh->mNumBones; bone_index++)
		{
			aiBone* pBone = pMesh->mBones[bone_index];
			AddAiBone(pBone);
		}
	}

	BuildSkeleton(m_pScene->mRootNode, -1);

	this->ProcessNode(m_pScene->mRootNode, m_pScene, DirectX::XMMatrixIdentity());
	return true;
}

void Model::ProcessNode(aiNode * node,const aiScene * scene, const XMMATRIX& parentTransformMatirx)
{
	XMMATRIX nodeTransformMatrix = XMMatrixTranspose(XMMATRIX(&node->mTransformation.a1)) * parentTransformMatirx;
	

	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.emplace_back(this->ProcessMesh(mesh, scene, nodeTransformMatrix));
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		this->ProcessNode(node->mChildren[i], scene, nodeTransformMatrix);
	}
}

Mesh Model::ProcessMesh(aiMesh * mesh, const aiScene * scene, const XMMATRIX& transformMatirx)
{
	MeshParameters params;
	std::vector<DWORD> indices;
	Material material;

	if (mesh->HasPositions())
		params.position.reserve(mesh->mNumVertices);
	if (mesh->HasVertexColors(0))       
		params.color.reserve(mesh->mNumVertices);
	if (mesh->HasNormals())
		params.normal.reserve(mesh->mNumVertices);
	if (mesh->HasTextureCoords(0))
		params.texcoord.reserve(mesh->mNumVertices);
	if (mesh->HasTangentsAndBitangents())
		params.tangent.reserve(mesh->mNumVertices);
	if (mesh->HasTangentsAndBitangents())
		params.bitangent.reserve(mesh->mNumVertices);
	if (mesh->HasBones())
		params.bone_names.resize(mesh->mNumVertices, { 0, 0, 0, 0 });
	if (mesh->HasBones())                 
		params.bone_weights.resize(mesh->mNumVertices, { 0.0f, 0.0f, 0.0f, 0.0f });

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		DirectX::XMFLOAT3 position;
		position.x = mesh->mVertices[i].x;
		position.y = mesh->mVertices[i].y;
		position.z = mesh->mVertices[i].z;
		params.position.emplace_back(position);

		DirectX::XMFLOAT3 normal;
		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;
		params.normal.emplace_back(normal);

		DirectX::XMFLOAT2 texcoord;
		if (mesh->HasTextureCoords(0))
		{
			texcoord.x = mesh->mTextureCoords[0][i].x;
			texcoord.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			texcoord = { 0.0f, 0.0f };
		}
		params.texcoord.emplace_back(texcoord);

		if (mesh->HasTangentsAndBitangents())
		{
			DirectX::XMFLOAT3 tangent;
			tangent.x = mesh->mTangents[i].x;
			tangent.y = mesh->mTangents[i].y;
			tangent.z = mesh->mTangents[i].z;
			params.tangent.emplace_back(tangent);

			DirectX::XMFLOAT3 bitangent;
			bitangent.x = mesh->mBitangents[i].x;
			bitangent.y = mesh->mBitangents[i].y;
			bitangent.z = mesh->mBitangents[i].z;
			params.bitangent.emplace_back(bitangent);
		}

		if (mesh->HasVertexColors(0))
		{
			DirectX::XMFLOAT4 color;
			color.x = mesh->mColors[0]->r;
			color.y = mesh->mColors[0]->g;
			color.z = mesh->mColors[0]->b;
			color.w = mesh->mColors[0]->a;
			params.color.emplace_back(color);
		}
	}


	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	//
	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		aiBone* pBone = mesh->mBones[i];
		int bone_index = FindBoneByName(pBone->mName.C_Str());
		assert(bone_index != -1, "referencing invalid bone");
		for (unsigned int weight_index = 0; weight_index < pBone->mNumWeights; weight_index++)
		{
			const aiVertexWeight& pWeight = pBone->mWeights[weight_index];
			AddBoneWeight(&params.bone_names, &params.bone_weights, pWeight.mVertexId, bone_index, pWeight.mWeight);
		}
	}

	aiMaterial* pMaterial = scene->mMaterials[mesh->mMaterialIndex];
	LoadMaterialTextures(material, pMaterial, aiTextureType::aiTextureType_DIFFUSE, scene);
	LoadMaterialTextures(material, pMaterial, aiTextureType::aiTextureType_NORMALS, scene);
	LoadMaterialTextures(material, pMaterial, aiTextureType::aiTextureType_SPECULAR, scene);
	LoadMaterialTextures(material, pMaterial, aiTextureType::aiTextureType_EMISSIVE, scene);
	LoadMaterialTextures(material, pMaterial, aiTextureType::aiTextureType_AMBIENT, scene);

	float shininess = 0.0f;
	pMaterial->Get(AI_MATKEY_SHININESS, shininess);
	if (shininess <= 1.0f)
	{
		shininess = 32.0f;
	}
	material.SetShininess(shininess);

	return Mesh(this->device, this->deviceContext, *cb_ps_material, params, indices, material, transformMatirx);
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
	// check if texture is an embedded texture but no indexed (path will be the texture's name instead of #)
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
		case aiTextureType_AMBIENT:
			pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, aiColor);
			material.SetAmbientColour(aiColor.r, aiColor.g, aiColor.b);
			break;
		case aiTextureType_DIFFUSE:
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
			material.SetDiffuseColour(aiColor.r, aiColor.g, aiColor.b);
			break;
		case aiTextureType_SPECULAR:
			pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, aiColor);
			if (aiColor.IsBlack())
			{
				material.SetSpecularColour(1.0f, 1.0f, 1.0f);
			}
			else
			{
				material.SetSpecularColour(aiColor.r, aiColor.g, aiColor.b);
			}
			break;
		case aiTextureType_EMISSIVE:
			pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, aiColor);
			material.SetEmissiveColour(aiColor.r, aiColor.g, aiColor.b);
			break;
		case aiTextureType_NORMALS:
		case aiTextureType_HEIGHT:
			return; 
		default:
			assert(false, "Unknown texture type");
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
}

int Model::GetTextureIndex(aiString * pStr)
{
	assert(pStr->length >= 2);

	return atoi(&pStr->C_Str()[1]);
}

int Model::AddBone(aiNode * node, aiBone * bone, int parent_index)
{
	int node_index = AddSkeletonNode(node, parent_index);

	BoneData boneData;
	boneData.index = node_index;
	boneData.name = bone->mName.C_Str();
	boneData.inverse_transform = AiToDxMatrix(bone->mOffsetMatrix);

	m_mapBoneNameToIndex[boneData.name] = (int)m_Bones.size();

	m_Bones.push_back(boneData);

	return node_index;
}

void Model::AddAiBone(aiBone * pBone)
{
	m_mapBoneNameToAiBone[pBone->mName.C_Str()] = pBone;
}

int Model::FindBoneByName(const std::string & name) const
{
	auto it = m_mapBoneNameToIndex.find(name);
	if (it == m_mapBoneNameToIndex.end())
	{
		return -1;
	}
	return it->second;
}

int Model::FindNodeByName(const std::string & name) const
{
	auto it = m_mapNodeNameToIndex.find(name);
	if (it == m_mapNodeNameToIndex.end())
		return -1;
	return it->second;
}

void Model::AddBoneWeight(std::vector<XMFLOAT4>* ids, std::vector<XMFLOAT4>* weights, unsigned int vertex_id, unsigned int bone_id, float weight)
{
	XMFLOAT4& curr_id = (*ids)[vertex_id];
	XMFLOAT4& curr_weight = (*weights)[vertex_id];

	if (curr_weight.x == 0.0f)
	{
		curr_id.x = bone_id;
		curr_weight.x = weight;
	}
	else if (curr_weight.y == 0.0f)
	{
		curr_id.y = bone_id;
		curr_weight.y = weight;
	}
	else if (curr_weight.z == 0.0f)
	{
		curr_id.z = bone_id;
		curr_weight.z = weight;
	}
	else if (curr_weight.w == 0.0f)
	{
		curr_id.w = bone_id;
		curr_weight.w = weight;
	}
	else
	{
		assert(false, "bone weights only support 0~4");
	}
}

int Model::AddSkeletonNode(aiNode * node, int parent_index)
{
	int index = (int)m_OriginalSkeleton.size();
	m_mapNodeNameToIndex[node->mName.C_Str()] = index;

	BoneNode boneNode;
	boneNode.local_transform = AiToDxMatrix(node->mTransformation);
	boneNode.parent_index = parent_index;

	m_OriginalSkeleton.push_back(boneNode);

	return index;
}

void Model::BuildSkeleton(aiNode * pNode, int parent_index)
{
	aiBone* pBone = GetAiBoneByName(pNode->mName.C_Str());

	int index;
	if (!pBone)
	{
		index = AddSkeletonNode(pNode, parent_index);
	}
	else
	{
		index = AddBone(pNode, pBone, parent_index);
	}
	for (unsigned int i = 0; i < pNode->mNumChildren; i++)
	{
		BuildSkeleton(pNode->mChildren[i], index);
	}
}

aiBone * Model::GetAiBoneByName(const std::string & name)
{
	auto it = m_mapBoneNameToAiBone.find(name);
	if (it == m_mapBoneNameToAiBone.end())
	{
		return nullptr;
	}
	return it->second;
}

void Model::LoadAnimations(MeshAnimator* animator_out, ConstantBuffer<CB_Bones>* cbufBone, AnimationComponent* animComp)
{
	m_Animations.reserve(m_pScene->mNumAnimations);

	for (unsigned int anim_index = 0; anim_index < m_pScene->mNumAnimations; anim_index++)
	{
		aiAnimation* pAnimation = m_pScene->mAnimations[anim_index];

		float ticks_per_second = (float)pAnimation->mTicksPerSecond;
		if (directory.find(".gltf") != std::string::npos || directory.find(".glb") != std::string::npos)
		{
			ticks_per_second = 1000.0f;
		}
		if (ticks_per_second == 0.0f)
		{
			ticks_per_second = 1.0f;
		}

		MeshAnimation animation;
		animation.name = pAnimation->mName.C_Str();
		animation.channels.resize(pAnimation->mNumChannels);
		animation.duration = (float)pAnimation->mDuration / ticks_per_second;

		for (unsigned int channel_index = 0; channel_index < pAnimation->mNumChannels; channel_index++)
		{
			aiNodeAnim* pNodeAnim = pAnimation->mChannels[channel_index];
			int node_index = FindNodeByName(pNodeAnim->mNodeName.C_Str());
			if (node_index == -1)
			{
				COM_ERROR_IF_FAILED(-1, "Missing animated node.");
				continue;
			}

			AnimationChannel& channel = animation.channels[channel_index];
			channel.node_index = node_index;

			channel.position_keyframes.reserve(pNodeAnim->mNumPositionKeys);
			for (unsigned int keyframe_index = 0; keyframe_index < pNodeAnim->mNumPositionKeys; keyframe_index++)
			{
				const aiVectorKey& ai_key = pNodeAnim->mPositionKeys[keyframe_index];

				PosKeyFrame keyframe;
				keyframe.timestamp = (float)ai_key.mTime / ticks_per_second;
				keyframe.value.x = ai_key.mValue.x;
				keyframe.value.y = ai_key.mValue.y;
				keyframe.value.z = ai_key.mValue.z;

				channel.position_keyframes.push_back(keyframe);
			}

			channel.rotation_keyframes.reserve(pNodeAnim->mNumRotationKeys);
			for (unsigned int keyframe_index = 0; keyframe_index < pNodeAnim->mNumRotationKeys; keyframe_index++)
			{
				const aiQuatKey& ai_key = pNodeAnim->mRotationKeys[keyframe_index];

				RotKeyFrame keyframe;
				keyframe.timestamp = (float)ai_key.mTime / ticks_per_second;
				keyframe.value.x = ai_key.mValue.x;
				keyframe.value.y = ai_key.mValue.y;
				keyframe.value.z = ai_key.mValue.z;
				keyframe.value.w = ai_key.mValue.w;

				channel.rotation_keyframes.push_back(keyframe);
			}
			animComp->AddChannel(animation.name, channel_index);
		}
		m_Animations.push_back(animation);
	}
	*animator_out = MeshAnimator(std::move(m_OriginalSkeleton), std::move(m_Bones), std::move(m_Animations), cbufBone);
}
