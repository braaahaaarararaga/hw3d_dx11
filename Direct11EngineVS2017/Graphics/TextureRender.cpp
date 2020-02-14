#include "TextureRender.h"
#include "../COMException.h"
#include "..\\d3dUtil.h"

TextureRender::TextureRender(ID3D11Device * device, int width, int height, TexFormat format, bool generateMips)
{
	Resize(device, width, height, format, generateMips);
}

void TextureRender::Resize(ID3D11Device * device, int width, int height, TexFormat format, bool generateMips)
{
	outputTextureSRV.Reset();
	outputTextureRTV.Reset();
	outputTextureDSV.Reset();

	cacheRTV.Reset();
	cacheDSV.Reset();

	this->generateMips = generateMips;

	HRESULT hr;

	ComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = (generateMips ? 0 : 1);
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = (DXGI_FORMAT)format;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	hr = device->CreateTexture2D(&texDesc, nullptr, texture.ReleaseAndGetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create RTT texture.");

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	hr = device->CreateRenderTargetView(texture.Get(), &rtvDesc, outputTextureRTV.ReleaseAndGetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create RTT RTV");

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, outputTextureSRV.ReleaseAndGetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create RTT SRV");

	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> depthTex;
	hr = device->CreateTexture2D(&texDesc, nullptr, depthTex.ReleaseAndGetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create dpth texture");

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = texDesc.Format;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = device->CreateDepthStencilView(depthTex.Get(), &dsvDesc,
		outputTextureDSV.ReleaseAndGetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create DSV");

	outputViewPort.TopLeftX = 0.0f;
	outputViewPort.TopLeftY = 0.0f;
	outputViewPort.Width = static_cast<float>(width);
	outputViewPort.Height = static_cast<float>(height);
	outputViewPort.MinDepth = 0.0f;
	outputViewPort.MaxDepth = 1.0f;
}

void TextureRender::Begin(ID3D11DeviceContext * deviceContext)
{
	deviceContext->OMGetRenderTargets(1, cacheRTV.GetAddressOf(), cacheDSV.GetAddressOf());
	UINT num_Viewports = 1;
	deviceContext->RSGetViewports(&num_Viewports, &cacheViewPort);

	float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	deviceContext->ClearRenderTargetView(outputTextureRTV.Get(), black);
	deviceContext->ClearDepthStencilView(outputTextureDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	deviceContext->OMSetRenderTargets(1, outputTextureRTV.GetAddressOf(), outputTextureDSV.Get());
	deviceContext->RSSetViewports(1, &outputViewPort);
}

void TextureRender::End(ID3D11DeviceContext * deviceContext)
{
	deviceContext->RSSetViewports(1, &cacheViewPort);
	deviceContext->OMSetRenderTargets(1, cacheRTV.GetAddressOf(), cacheDSV.Get());

	if (generateMips)
	{
		deviceContext->GenerateMips(outputTextureSRV.Get());
	}

	cacheDSV.Reset();
	cacheRTV.Reset();
}

ID3D11ShaderResourceView * TextureRender::GetOutputTexture()
{
	return outputTextureSRV.Get();
}

void TextureRender::SetDebugObjectName(const std::string & name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	D3D11SetDebugObjectName(outputTextureDSV.Get(), name + ".TextureDSV");
	D3D11SetDebugObjectName(outputTextureSRV.Get(), name + ".TextureSRV");
	D3D11SetDebugObjectName(outputTextureRTV.Get(), name + ".TextureRTV");
#else
	UNREFERENCED_PARAMETER(name);
#endif
}
