#include "Light.h"

bool Light::Initialize(ID3D11Device * device, ID3D11DeviceContext * deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader,
	ConstantBuffer<CB_PS_material>& cb_ps_material, IVertexShader * pVertexShader)
{
	if (!model.Initialize("Data\\Objects\\sphere.gltf", device, deviceContext, cb_vs_vertexshader, cb_ps_material, pVertexShader))
		return false;

	SetPosition(0.0f, 0.0f, 0.0f);
	SetRotation(0.0f, 0.0f, 0.0f);
	SetScale(0.25f, 0.25f, 0.25f);
	UpdateMatrix();

	this->device = device;
	this->deviceContext = deviceContext;

	// shadow initialize
	shadowMapRTT = std::make_unique<ShadowMapRTT>(device, shadow_width, shadow_height);

	float aspect = (float)shadow_width / (float)shadow_height;
	projMat = XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), aspect, 0.1f, 800.0f);
	viewMat = XMMatrixLookAtLH(GetPositionVector(), GetPositionVector() + GetForwardVector(), { 0.0f, 1.0f, 0.0f });

	return true;
}

DirectX::XMMATRIX Light::GetVpMatrix()
{
	return viewMat * projMat;
}

void Light::SetUpShadowMap()
{
	shadowMapRTT->SetUpToRender(deviceContext);
}

void Light::BindShadowResourceView()
{
	shadowMapRTT->BindTexture(deviceContext, 4);
}

void Light::UpdateMatrix()
{
	RenderableGameObject::UpdateMatrix();
	viewMat = XMMatrixLookAtLH(GetPositionVector(), GetPositionVector() + GetForwardVector(), { 0.0f, 1.0f, 0.0f });
}
