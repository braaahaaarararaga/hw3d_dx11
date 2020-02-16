#include "Graphics/Graphics.h"
#include "ResourceManager.h"


template<typename T>
using ResourceMap = std::unordered_map<std::string, Resource<T>>;
template<typename T>
using UniqueResourceMap = std::unordered_map<std::string, std::unique_ptr<T>>;

static ResourceMap<Texture>        g_mapTextures;
static UniqueResourceMap<IVertexShader>   g_mapVShaders;
static UniqueResourceMap<IPixelShader>  g_mapPShaders;

Resource<Texture> ResourceManager::GetTexture(ID3D11Device* device, const std::string& filename, aiTextureType type)
{
	auto it = g_mapTextures.find(filename);
	if (it == g_mapTextures.end())
	{
		auto pResource = std::make_shared<Texture>(device, StringHelper::StringToWide(filename), type);
		g_mapTextures[filename] = pResource;
		return pResource;
	}

	return it->second;
}

Resource<Texture> ResourceManager::GetTexture(ID3D11Device * device, const std::string & texturename, const char * pData, size_t size, aiTextureType type)
{
	auto it = g_mapTextures.find(texturename);
	if (it == g_mapTextures.end())
	{
		auto pResource = std::make_shared<Texture>(device, pData, size, type);
		g_mapTextures[texturename] = pResource;
		return pResource;
	}

	return it->second;
}

Resource<Texture> ResourceManager::GetTexture(ID3D11Device * device, const std::string & texturename, const Color & color, aiTextureType type)
{
	auto it = g_mapTextures.find(texturename);
	if (it == g_mapTextures.end())
	{
		auto pResource = std::make_shared<Texture>(device, color, type);
		g_mapTextures[texturename] = pResource;
		return pResource;
	}

	return it->second;
}

Resource<Texture> ResourceManager::GetTexture(ID3D11Device * device, const std::string & texturename, const Color * colorData, UINT width, UINT height, aiTextureType type)
{
	auto it = g_mapTextures.find(texturename);
	if (it == g_mapTextures.end())
	{
		auto pResource = std::make_shared<Texture>(device, colorData, width, height, type);
		g_mapTextures[texturename] = pResource;
		return pResource;
	}

	return it->second;
}

static std::string GetShaderHashName(const std::string& filename, const std::vector<ShaderMacro>& macros)
{
	std::string name = filename;
	for (const auto& macro : macros)
	{
		name += macro.name;
		name += macro.value;
	}
	return name;
}

IVertexShader * ResourceManager::GetVertexShader(const std::string & filename, Graphics * gfx, const std::vector<ShaderMacro>& macros)
{
	std::string name = GetShaderHashName(filename, macros);
	auto it = g_mapVShaders.find(name);
	if (it == g_mapVShaders.end())
	{
		auto pResource = gfx->CreateVertexShader(filename, macros);
		g_mapVShaders[name] = std::unique_ptr<IVertexShader>(pResource);
		return pResource;
	}

	return it->second.get();
}

IPixelShader * ResourceManager::GetPixelShader(const std::string & filename, Graphics * gfx, const std::vector<ShaderMacro>& macros)
{
	std::string name = GetShaderHashName(filename, macros);
	auto it = g_mapPShaders.find(name);
	if (it == g_mapPShaders.end())
	{
		auto pResource = gfx->CreatePixelShader(filename, macros);
		g_mapPShaders[name] = std::unique_ptr<IPixelShader>(pResource);
		return pResource;
	}

	return it->second.get();
}



template <typename T>
static void CleanUpResources(std::unordered_map<std::string, std::shared_ptr<T>>& resource_map)
{
	for (auto i = resource_map.begin(); i != resource_map.end(); )
	{
		if (i->second.use_count() == 1)
		{
			i = resource_map.erase(i);
		}
		else
		{
			++i;
		}
	}
}

std::vector<ShaderMacro> ResourceManager::BuildMacrosForMesh(const Mesh & mesh)
{
	std::vector<ShaderMacro> macros;
	if (mesh.HasTangentsAndBitangents())
	{
		macros.emplace_back("HAS_TANGENT");
	}
	if (mesh.HasBones())
	{
		macros.emplace_back("HAS_BONES");
	}
	return macros;
}

void ResourceManager::CleanUp()
{
	CleanUpResources(g_mapTextures);

	g_mapVShaders.clear();
	g_mapPShaders.clear();
}

