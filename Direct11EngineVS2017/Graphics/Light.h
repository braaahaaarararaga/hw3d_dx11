#pragma once
#include "RenderableGameObject.h"
#include "ShadowMapRTT.h"

class Light : public RenderableGameObject
{
public:
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader,
		ConstantBuffer<CB_PS_material>& cb_ps_material, Graphics * gfx);

	DirectX::XMFLOAT3 lightColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	float lightStrenght = 3.5f;
	float attenuation_a = 1.0f;
	float attenuation_b = 0.02f;
	float attenuation_c = 0.01f;

	DirectX::XMMATRIX GetVpMatrix();
	void SetUpShadowMap();
	void BindShadowResourceView();

protected:
	void UpdateMatrix() override;

private:
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	const int shadow_width = 1024;
	const int shadow_height = 1024;

	std::unique_ptr<ShadowMapRTT> shadowMapRTT;
	DirectX::XMMATRIX viewMat;
	DirectX::XMMATRIX projMat;

};

