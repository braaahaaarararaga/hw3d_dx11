#pragma once
#include <string>
#include "Graphics/Graphics.h"

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
	static Resource<Texture>        GetTexture(const std::string& filename);
	static Resource<IVertexShader>   GetVertexShader(const std::string& filename, Graphics& gfx);
	//static Resource<IPixelShader>    GetPixelShader(const std::string& filename);

	static void CleanUp();
};

