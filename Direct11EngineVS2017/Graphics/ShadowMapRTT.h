#pragma once
#include <d3d11.h>
#include <memory>
#include <wrl/client.h>

class ShadowMapRTT
{
public:
	ShadowMapRTT() = default;
	ShadowMapRTT(ID3D11Device* device, int width, int height);
	ShadowMapRTT(const ShadowMapRTT&) = delete;
	ShadowMapRTT& operator = (ShadowMapRTT&) = delete;

public:
	void Resize(ID3D11Device* device, int width, int height);
	void SetUpToRender(ID3D11DeviceContext* deviceContext);
	void BindTexture(ID3D11DeviceContext* deviceContext, UINT slot);
	ID3D11ShaderResourceView* const* GetShaderResourceView();

private:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<ID3D11ShaderResourceView> shadowmap_resourceView;
	std::unique_ptr<CD3D11_VIEWPORT> shadowmap_viewport;
	ComPtr<ID3D11Texture2D> shadowmap_depthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> shadowmap_depthStencilView;
};

