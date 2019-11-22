#pragma once
#include <string>
#include "Graphics/Graphics.h"
#include "Texture.h"

class Texture;
//class Colour;
//class Mesh;
class IVertexShader;
//class IPixelShader;


template <typename T>
using Resource = std::shared_ptr<T>;

class ResourceManager
{
public:
	static Resource<Texture>        GetTexture(ID3D11Device* device, const std::string& filename, aiTextureType type);
	static Resource<Texture>        GetTexture(ID3D11Device* device, const std::string& texturename, const char* pData, size_t size, aiTextureType type);
	static Resource<Texture>        GetTexture(ID3D11Device* device, const std::string& texturename, const Color * colorData, UINT width, UINT height, aiTextureType type);
	static Resource<IVertexShader>   GetVertexShader(const std::string& filename, Graphics& gfx);
	//static Resource<IPixelShader>    GetPixelShader(const std::string& filename);

	static void CleanUp();
};

