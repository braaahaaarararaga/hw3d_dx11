#pragma once
#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Material.h"
#include "Mesh.h"
#include "AnimationSkeleton.h"
#include <unordered_map>

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
		 ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader,ConstantBuffer<CB_PS_material>& cb_ps_material, Graphics * gfx);
	void Draw(const XMMATRIX & world, const XMMATRIX & viewProjectionMatrix);
	bool InitAnimation(ConstantBuffer<CB_Bones>* cbufBone, MeshAnimator* animator_out, AnimationComponent* animComp);

	void SetMeshDiffuseColor(Color& color);
	
private:
	std::vector<Mesh> meshes;
	bool LoadModel(const std::string& filePath);
	void ProcessNode(aiNode* node,const aiScene* scene, const XMMATRIX& parentTransformMatirx);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatirx);
	TextureStorageType DetermineTextureStorageType(const aiScene* scene, aiMaterial* pMat, unsigned int index, aiTextureType textureType);
	void LoadMaterialTextures(Material& material, aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene);
	int GetTextureIndex(aiString* pStr);

	int AddBone(aiNode* node, aiBone* bone, int parent_index);
	void AddAiBone(aiBone* pBone);
	int FindBoneByName(const std::string& name) const;
	int FindNodeByName(const std::string& name) const;
	void AddBoneWeight(std::vector<XMUINT4>* ids, std::vector<XMFLOAT4>* weights, unsigned int vertex_id,
		unsigned int bone_id, float weight);
	int AddSkeletonNode(aiNode* node, int parent_index);
	void BuildSkeleton(aiNode* pNode, int parent_index);
	aiBone* GetAiBoneByName(const std::string& name);

	void LoadAnimations(MeshAnimator* animator_out, ConstantBuffer<CB_Bones>* cbufBone, AnimationComponent* animComp);

	Assimp::Importer m_Importer;
	const aiScene* m_pScene;

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	ConstantBuffer<CB_VS_vertexshader>* cb_vs_vertexshader = nullptr;
	ConstantBuffer<CB_PS_material>* cb_ps_material = nullptr;
	Graphics * gfx = nullptr;
	std::string directory = "";

	std::vector<BoneData> m_Bones;
	std::vector<BoneNode> m_OriginalSkeleton;
	std::unordered_map<std::string, aiBone*> m_mapBoneNameToAiBone;
	std::unordered_map<std::string, int> m_mapBoneNameToIndex;
	std::unordered_map<std::string, int> m_mapNodeNameToIndex;

	std::vector<MeshAnimation> m_Animations;
};