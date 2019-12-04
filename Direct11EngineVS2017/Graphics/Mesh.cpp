#include "Mesh.h"

Mesh::Mesh(ID3D11Device * device, ID3D11DeviceContext * deviceContext, std::vector<Vertex>& vertices, std::vector<DWORD>& indices, Material& material, const DirectX::XMMATRIX& matrixTransform)
{
	this->deviceContext = deviceContext;
	this->material = material;
	this->transformMatrix = matrixTransform;

	HRESULT hr = this->vertexbuffer.Initialize(device, vertices.data(), vertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");

	hr = this->indexBuffer.Initialize(device, indices.data(), indices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");
}

Mesh::Mesh(const Mesh & mesh)
{
	this->deviceContext = mesh.deviceContext;
	this->indexBuffer = mesh.indexBuffer;
	this->vertexbuffer = mesh.vertexbuffer;
	this->material = mesh.material;
	this->transformMatrix = mesh.transformMatrix;
}

void Mesh::Draw()
{
	UINT offset = 0;
	
	this->deviceContext->PSSetShaderResources(0, 1, material.GetDiffuseTexture()->GetShaderResourceView());

	this->deviceContext->IASetVertexBuffers(0, 1, this->vertexbuffer.GetAddressOf(), this->vertexbuffer.StridePtr(), &offset);
	this->deviceContext->IASetIndexBuffer(this->indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(this->indexBuffer.IndexCount(), 0, 0);
}

const DirectX::XMMATRIX & Mesh::GetTransformMatrix()
{
	return transformMatrix;
}
