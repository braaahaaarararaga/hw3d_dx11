#pragma once
#include <string>
#include <d3d11.h>
#include "Graphics//Texture.h"
#include <vector>
#include "Graphics//VertexShader.h"

class Mesh;
class Graphics;
//class IPixelShader;


template <typename T>
using Resource = std::shared_ptr<T>;

class ResourceManager
{
public:
	static Resource<Texture>        GetTexture(ID3D11Device* device, const std::string& filename, aiTextureType type);
	static Resource<Texture>        GetTexture(ID3D11Device* device, const std::string& texturename, const char* pData, size_t size, aiTextureType type);
	static Resource<Texture>        GetTexture(ID3D11Device* device, const std::string& texturename, const Color& color, aiTextureType type);
	static Resource<Texture>        GetTexture(ID3D11Device* device, const std::string& texturename, const Color * colorData, UINT width, UINT height, aiTextureType type);
	static IVertexShader*   GetVertexShader(const std::string& filename, Graphics* gfx, const std::vector<ShaderMacro>& macros = {});
	//static Resource<IPixelShader>    GetPixelShader(const std::string& filename);

	static std::vector<ShaderMacro> BuildMacrosForMesh(const Mesh& mesh);

	static void CleanUp();
};

