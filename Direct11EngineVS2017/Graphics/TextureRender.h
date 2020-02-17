#pragma once
#include <d3d11.h>
#include <memory>
#include <wrl/client.h>
#include <string>
#include "TextureFormat.h"

class TextureRender
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	TextureRender() = default;
	TextureRender(ID3D11Device* device, int width, int height, TexFormat format = TEX_FORMAT_R8G8B8A8_UNORM, bool useDepth = true, bool generateMips = false);
	TextureRender(const TextureRender&) = delete;
	TextureRender& operator = (TextureRender&) = delete;

public:
	void Resize(ID3D11Device* device, int width, int height, TexFormat format = TEX_FORMAT_R8G8B8A8_UNORM, bool useDepth = true, bool generateMips = false);
	void Begin(ID3D11DeviceContext * deviceContext);
	void End(ID3D11DeviceContext * deviceContext);
	ID3D11ShaderResourceView * const * GetOutputTexture();

	void SetDebugObjectName(const std::string& name);

private:
	ComPtr<ID3D11ShaderResourceView>    outputTextureSRV;
	ComPtr<ID3D11RenderTargetView>      outputTextureRTV;
	ComPtr<ID3D11DepthStencilView>      outputTextureDSV;
	D3D11_VIEWPORT                      outputViewPort;


	ComPtr<ID3D11RenderTargetView>      cacheRTV;        
	ComPtr<ID3D11DepthStencilView>      cacheDSV;        
	D3D11_VIEWPORT                      cacheViewPort;     

	bool generateMips;
	bool useDepth;
};

