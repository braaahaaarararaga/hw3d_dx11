#include "ResourceManager.h"

#include "Graphics/Texture.h"
#include "VertexShader.h"
#include "StringHelper.h"


template<typename T>
using ResourceMap = std::unordered_map<std::string, Resource<T>>;

static ResourceMap<Texture>        g_mapTextures;
static ResourceMap<IVertexShader>   g_mapVShaders;

Resource<Texture> ResourceManager::GetTexture(const std::string& filename)
{
	auto it = g_mapTextures.find(filename);
	if (it == g_mapTextures.end())
	{
		auto pResource = std::make_shared<Texture>(StringHelper::StringToWide(filename));
		g_mapTextures[filename] = pResource;
		return pResource;
	}

	return it->second;
}

Resource<IVertexShader> ResourceManager::GetVertexShader(const std::string& filename, Graphics& gfx)
{
	auto it = g_mapVShaders.find(filename);
	if (it == g_mapVShaders.end())
	{
		auto pResource = std::shared_ptr<IVertexShader>(gfx.CreateVertexShader(filename));
		g_mapVShaders[filename] = pResource;
		return pResource;
	}

	return it->second;
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

void ResourceManager::CleanUp()
{
	CleanUpResources(g_mapTextures);
	CleanUpResources(g_mapVShaders);
}

