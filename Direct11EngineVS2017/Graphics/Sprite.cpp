﻿#include "Sprite.h"



bool Sprite::Initialize(ID3D11Device * device, ID3D11DeviceContext * deviceContext, float width, float height, std::string spritePath, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_vertexshader_2d)
{
	this->deviceContext = deviceContext;
	if (deviceContext == nullptr)
		return false;
	this->cb_vs_vertexshader_2d = &cb_vs_vertexshader_2d;

	texture = ResourceManager::GetTexture(device, spritePath, aiTextureType::aiTextureType_DIFFUSE);

	std::vector<Vertex2D> vertexData =
	{
		Vertex2D(-0.5f, -0.5f, 0.0f, 0.0f, 0.0f), //Topleft
		Vertex2D(0.5f, -0.5f, 0.0f, 1.0f, 0.0f), //TopRight
		Vertex2D(-0.5f, 0.5f, 0.0f, 0.0f, 1.0f),  //BottomLeft
		Vertex2D(0.5f, 0.5f, 0.0f, 1.0f, 1.0f),
	};

	std::vector<DWORD> indexData =
	{
		0,1,2,
		2,1,3
	};

	HRESULT hr = vertices.Initialize(device, vertexData.data(), vertexData.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize 2d sprite vertex buffer.");

	hr = indices.Initialize(device, indexData.data(), indexData.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize 2d sprite index buffer.");

	SetPosition(0.0f, 0.0f, 0.0f);
	SetRotation(0.0f, 0.0f, 0.0f);

	SetScale(width, height);

	return true;

}

void Sprite::Draw(XMMATRIX orthoMatrix)
{
	XMMATRIX mvpMatrix = worldMatrix * orthoMatrix;
	deviceContext->VSSetConstantBuffers(0, 1, cb_vs_vertexshader_2d->GetAddressOf());
	cb_vs_vertexshader_2d->data.wvpMatrix = XMMatrixTranspose(mvpMatrix);
	cb_vs_vertexshader_2d->ApplyChanges();

	deviceContext->PSSetShaderResources(0, 1, texture->GetShaderResourceView());

	const UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, vertices.GetAddressOf(), vertices.StridePtr(), &offsets);
	deviceContext->IASetIndexBuffer(indices.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexed(indices.IndexCount(), 0, 0);
}

float Sprite::GetWidth()
{
	return scale.x;
}

float Sprite::GetHeight()
{
	return scale.y;
}

void Sprite::UpdateMatrix()
{
	worldMatrix = XMMatrixScaling(scale.x, scale.y, 1.0f) * XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) * XMMatrixTranslation(pos.x + scale.x / 2.0f, pos.y + scale.y / 2.0f, pos.z);
}
