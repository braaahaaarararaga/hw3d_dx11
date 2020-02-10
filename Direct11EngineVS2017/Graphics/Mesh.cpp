#include "Mesh.h"
#include "VertexShader.h"

Mesh::Mesh(ID3D11Device * device, ID3D11DeviceContext * deviceContext, ConstantBuffer<CB_PS_material>& cb_ps_material, MeshParameters& params, std::vector<DWORD>& indices, Material& material, const DirectX::XMMATRIX& matrixTransform)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->cb_ps_material = &cb_ps_material;
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
	this->cb_ps_material = mesh.cb_ps_material;
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
	if (pVertexShader->RequiresVertexAttribute(AttributeInfo::VertexAttribute::BoneId))
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
	if (pVertexShader->RequiresVertexAttribute(AttributeInfo::VertexAttribute::UV))
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
	


}

void Mesh::BindMaterial() const
{
	cb_ps_material->data.GlobalAmbient = { 1.0f, 1.0f, 1.0f, 1.0f };
	cb_ps_material->data.AmbientColor = { 0.005f, 0.005f, 0.005f, 1.0f };
	cb_ps_material->data.DiffuseColor = DirectX::XMFLOAT4(material.GetDiffuseColor().x, material.GetDiffuseColor().y, material.GetDiffuseColor().z, 1.0f);
	cb_ps_material->data.SpecularColor = DirectX::XMFLOAT4(material.GetSpecularColor().x, material.GetSpecularColor().y, material.GetSpecularColor().z, 1.0f); 
	cb_ps_material->data.EmissiveColor = DirectX::XMFLOAT4(material.GetEmissiveColor().x, material.GetEmissiveColor().y, material.GetEmissiveColor().z, 1.0f);
	cb_ps_material->data.Opacity = material.GetOpacity();
	cb_ps_material->data.HasAmbientTexture = (material.GetAmbientTexture() != nullptr);
	cb_ps_material->data.HasDiffuseTexture = (material.GetDiffuseTexture() != nullptr);
	cb_ps_material->data.HasSpecularTexture = (material.GetSpecularTexture() != nullptr);
	cb_ps_material->data.HasEmissiveTexture = (material.GetEmissiveTexture() != nullptr);
	cb_ps_material->data.HasNormalTexture = (material.GetNormalTexture() != nullptr);
	cb_ps_material->data.HasBumpTexture = false;
	cb_ps_material->data.SpecularPower = material.GetShininess();

	cb_ps_material->ApplyChanges();
	deviceContext->PSSetConstantBuffers(2, 1, cb_ps_material->GetAddressOf());

	if (material.GetDiffuseTexture() != nullptr)
		this->deviceContext->PSSetShaderResources(0, 1, material.GetDiffuseTexture()->GetShaderResourceView());
	if (material.GetNormalTexture() != nullptr)
		this->deviceContext->PSSetShaderResources(1, 1, material.GetNormalTexture()->GetShaderResourceView());
	if (material.GetSpecularTexture() != nullptr)
		this->deviceContext->PSSetShaderResources(2, 1, material.GetSpecularTexture()->GetShaderResourceView());
	if (material.GetEmissiveTexture() != nullptr)
		this->deviceContext->PSSetShaderResources(3, 1, material.GetEmissiveTexture()->GetShaderResourceView());
}

void Mesh::SetDiffuseColor(Color& color)
{
	material.SetDiffuseColour((float)color.GetR() / 256.0f, (float)color.GetG() / 256.0f, (float)color.GetB() / 256.0f);
	//material.SetDiffuseColour(0.5f, 0.5f, 0.5f);
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
	BindMaterial();
	Bind(pVertexShader);
	this->deviceContext->IASetIndexBuffer(this->indexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(this->indexBuffer.IndexCount(), 0, 0);
}

const DirectX::XMMATRIX & Mesh::GetTransformMatrix()
{
	return transformMatrix;
}
