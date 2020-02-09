#include "ShadowMapRTT.h"
#include "../COMException.h"

ShadowMapRTT::ShadowMapRTT(ID3D11Device * device, int width, int height)
{
	Resize(device, width, height);
}

void ShadowMapRTT::Resize(ID3D11Device * device, int width, int height)
{
	HRESULT hr;
	// shadow map
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	hr = device->CreateTexture2D(&depthBufferDesc, NULL, shadowmap_depthStencilBuffer.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil buffer for shadow.");

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	depthStencilViewDesc.Flags = 0;
	hr = device->CreateDepthStencilView(shadowmap_depthStencilBuffer.Get(), &depthStencilViewDesc, shadowmap_depthStencilView.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil view for shadow.");

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = depthBufferDesc.MipLevels;
	hr = device->CreateShaderResourceView(shadowmap_depthStencilBuffer.Get(), &shaderResourceViewDesc, shadowmap_resourceView.GetAddressOf());

	// shadow viewport
	shadowmap_viewport = std::make_unique<CD3D11_VIEWPORT>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

}

void ShadowMapRTT::SetUpToRender(ID3D11DeviceContext * deviceContext)
{
	deviceContext->ClearDepthStencilView(shadowmap_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ID3D11RenderTargetView* shadowRenderTargetView[1] = { 0 };
	deviceContext->OMSetRenderTargets(1, shadowRenderTargetView, shadowmap_depthStencilView.Get());
	deviceContext->RSSetViewports(1, shadowmap_viewport.get());
	deviceContext->PSSetShader(NULL, NULL, 0);
}

void ShadowMapRTT::BindTexture(ID3D11DeviceContext * deviceContext, UINT slot)
{
	deviceContext->PSSetShaderResources(slot, 1, GetShaderResourceView());
}

ID3D11ShaderResourceView* const* ShadowMapRTT::GetShaderResourceView()
{
	return shadowmap_resourceView.GetAddressOf();
}
