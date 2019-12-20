#include "RenderableGameObject.h"

bool RenderableGameObject::Initialize(const std::string & filePath, ID3D11Device * device, ID3D11DeviceContext * deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, ConstantBuffer<CB_PS_material>& cb_ps_material, IVertexShader * pVertexShader)
{
	if (!model.Initialize(filePath, device, deviceContext, cb_vs_vertexshader, cb_ps_material, pVertexShader))
		return false;

	this->SetPosition(0.0f, 0.0f, 0.0f);
	this->SetRotation(0.0f, 0.0f, 0.0f);
	this->SetScale(1.0f, 1.0f, 1.0f);
	this->UpdateMatrix();
	return true;
}

bool RenderableGameObject::InitAnimation(ConstantBuffer<CB_Bones>& cbufBones)
{
	return model.InitAnimation(&cbufBones, animator_out.get());
}

void RenderableGameObject::Draw(const XMMATRIX & viewProjectionMatrix)
{
	
	model.Draw(this->worldMatrix, viewProjectionMatrix);
	
}

void RenderableGameObject::UpdateMatrix()
{
	this->worldMatrix = XMMatrixScaling(this->scale.x, this->scale.y, this->scale.z) * XMMatrixRotationRollPitchYaw(this->rot.x, this->rot.y, this->rot.z) * XMMatrixTranslation(this->pos.x, this->pos.y, this->pos.z);
	this->UpdateDirectionVectors();
}
