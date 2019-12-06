﻿#include "Light.h"

bool Light::Initialize(ID3D11Device * device, ID3D11DeviceContext * deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, IVertexShader * pVertexShader)
{
	if (!model.Initialize("Data\\Objects\\Sitting.fbx", device, deviceContext, cb_vs_vertexshader, pVertexShader))
		return false;

	this->SetPosition(0.0f, 0.0f, 0.0f);
	this->SetRotation(0.0f, 0.0f, 0.0f);
	this->SetScale(0.001f, 0.001f, 0.001f);
	this->UpdateMatrix();
	return true;
}
