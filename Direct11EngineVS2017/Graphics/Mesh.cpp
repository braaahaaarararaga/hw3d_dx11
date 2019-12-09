#include "Mesh.h"
#include "VertexShader.h"

Mesh::Mesh(ID3D11Device * device, ID3D11DeviceContext * deviceContext, MeshParameters& params, std::vector<DWORD>& indices, Material& material, const DirectX::XMMATRIX& matrixTransform)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->material = material;
	this->transformMatrix = matrixTransform;
	SetData(params);


	HRESULT hr = this->indexBuffer.Initialize(device, indices.data(), indices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");
}

Mesh::Mesh(const Mesh & mesh)
{
	this->device = mesh.device;
	this->deviceContext = mesh.deviceContext;
	this->indexBuffer = mesh.indexBuffer;
	this->vBufPosition = mesh.vBufPosition;
	this->vBufColor = mesh.vBufColor;
	this->vBufNormal = mesh.vBufNormal;
	this->vBufTexCoord = mesh.vBufTexCoord;
	this->vBufTangent = mesh.vBufTangent;
	this->vBufBitangent = mesh.vBufBitangent;
	this->vBufBoneNames = mesh.vBufBoneNames;
	this->vBufBoneWeights = mesh.vBufBoneWeights;
	this->material = mesh.material;
	this->transformMatrix = mesh.transformMatrix;
}

void Mesh::Bind(IVertexShader * pVertexShader) const
{
	size_t slot = 0;

	if (pVertexShader->RequiresVertexAttribute(AttributeInfo::VertexAttribute::Position))
	{
		assert(vBufPosition.VertexCount() > 0, "Shader requires model to have vertex position data");

		ID3D11Buffer* pVertexBuffer = vBufPosition.Get();
		UINT stride = (UINT)vBufPosition.Stride();
		UINT offset = 0;
		deviceContext->IASetVertexBuffers((UINT)slot, 1, &pVertexBuffer, &stride, &offset);
		slot++;
	}
	if (pVertexShader->RequiresVertexAttribute(AttributeInfo::VertexAttribute::Color))
	{
		assert(vBufColor.VertexCount() > 0, "Shader requires model to have vertex color data");
		ID3D11Buffer* pVertexBuffer = vBufColor.Get();
		UINT stride = (UINT)vBufColor.Stride();
		UINT offset = 0;
		deviceContext->IASetVertexBuffers((UINT)slot, 1, &pVertexBuffer, &stride, &offset);
		slot++;
	}
	if (pVertexShader->RequiresVertexAttribute(AttributeInfo::VertexAttribute::Normal))
	{
		assert(vBufNormal.VertexCount() > 0, "Shader requires model to have vertex normal data");
		ID3D11Buffer* pVertexBuffer = vBufNormal.Get();
		UINT stride = (UINT)vBufNormal.Stride();
		UINT offset = 0;
		deviceContext->IASetVertexBuffers((UINT)slot, 1, &pVertexBuffer, &stride, &offset);
		slot++;
	}
	if (pVertexShader->RequiresVertexAttribute(AttributeInfo::VertexAttribute::TexCoord))
	{
		assert(vBufTexCoord.VertexCount() > 0, "Shader requires model to have vertex UV data");
		ID3D11Buffer* pVertexBuffer = vBufTexCoord.Get();
		UINT stride = (UINT)vBufTexCoord.Stride();
		UINT offset = 0;
		deviceContext->IASetVertexBuffers((UINT)slot, 1, &pVertexBuffer, &stride, &offset);
		slot++;
	}
	if (pVertexShader->RequiresVertexAttribute(AttributeInfo::VertexAttribute::Tangent))
	{
		assert(vBufTangent.VertexCount() > 0, "Shader requires model to have vertex tangent data");
		ID3D11Buffer* pVertexBuffer = vBufTangent.Get();
		UINT stride = (UINT)vBufTangent.Stride();
		UINT offset = 0;
		deviceContext->IASetVertexBuffers((UINT)slot, 1, &pVertexBuffer, &stride, &offset);
		slot++;
	}
	if (pVertexShader->RequiresVertexAttribute(AttributeInfo::VertexAttribute::Bitangent))
	{
		assert(vBufBitangent.VertexCount() > 0, "Shader requires model to have vertex bitangent data");
		ID3D11Buffer* pVertexBuffer = vBufBitangent.Get();
		UINT stride = (UINT)vBufBitangent.Stride();
		UINT offset = 0;
		deviceContext->IASetVertexBuffers((UINT)slot, 1, &pVertexBuffer, &stride, &offset);
		slot++;
	}
	if (pVertexShader->RequiresVertexAttribute(AttributeInfo::VertexAttribute::BoneName))
	{
		assert(vBufBoneNames.VertexCount() > 0, "Shader requires model to have bone name data");
		ID3D11Buffer* pVertexBuffer = vBufBoneNames.Get();
		UINT stride = (UINT)vBufBoneNames.Stride();
		UINT offset = 0;
		deviceContext->IASetVertexBuffers((UINT)slot, 1, &pVertexBuffer, &stride, &offset);
		slot++;
	}
	if (pVertexShader->RequiresVertexAttribute(AttributeInfo::VertexAttribute::BoneWeight))
	{
		assert(vBufBoneWeights.VertexCount() > 0, "Shader requires model to have bone weight data");
		ID3D11Buffer* pVertexBuffer = vBufBoneWeights.Get();
		UINT stride = (UINT)vBufBoneWeights.Stride();
		UINT offset = 0;
		deviceContext->IASetVertexBuffers((UINT)slot, 1, &pVertexBuffer, &stride, &offset);
		slot++;
	}
}

void Mesh::SetData(MeshParameters& params)
{
	if (!params.position.empty())
	{
		vBufPosition.Initialize(device, params.position.data(), params.position.size());
	}
	if (!params.color.empty())
	{
		vBufColor.Initialize(device, params.color.data(), params.color.size());
	}
	if (!params.normal.empty())
	{
		vBufNormal.Initialize(device, params.normal.data(), params.normal.size());
	}
	if (!params.texcoord.empty())
	{
		vBufTexCoord.Initialize(device, params.texcoord.data(), params.texcoord.size());
	}
	if (!params.tangent.empty())
	{
		vBufTangent.Initialize(device, params.tangent.data(), params.tangent.size());
	}
	if (!params.bitangent.empty())
	{
		vBufBitangent.Initialize(device, params.bitangent.data(), params.bitangent.size());
	}
	if (!params.bone_names.empty())
	{
		vBufBoneNames.Initialize(device, params.bone_names.data(), params.bone_names.size());
	}
	if (!params.bone_weights.empty())
	{
		vBufBoneWeights.Initialize(device, params.bone_weights.data(), params.bone_weights.size());
	}
}

void Mesh::Draw(IVertexShader * pVertexShader)
{
	if (material.GetDiffuseTexture() != nullptr)
		this->deviceContext->PSSetShaderResources(0, 1, material.GetDiffuseTexture()->GetShaderResourceView());
	if (material.GetNormalTexture() != nullptr)
		this->deviceContext->PSSetShaderResources(1, 1, material.GetNormalTexture()->GetShaderResourceView());

	Bind(pVertexShader);
	this->deviceContext->IASetIndexBuffer(this->indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(this->indexBuffer.IndexCount(), 0, 0);
}

const DirectX::XMMATRIX & Mesh::GetTransformMatrix()
{
	return transformMatrix;
}
