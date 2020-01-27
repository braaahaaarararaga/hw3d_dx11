#include "RenderableGameObject.h"
#include "ImGui\\imgui.h"

bool RenderableGameObject::Initialize(const std::string & filePath, ID3D11Device * device, ID3D11DeviceContext * deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, ConstantBuffer<CB_PS_material>& cb_ps_material, IVertexShader * pVertexShader)
{
	this->deviceContext = deviceContext;
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
	mAnimComp = std::make_unique<AnimationComponent>(&mAnimator);
	mPlayAnimtion = true;
	mAnimTimer.Start();
	return model.InitAnimation(&cbufBones, &mAnimator, mAnimComp.get());
}

void RenderableGameObject::Draw(const XMMATRIX & viewProjectionMatrix)
{
	if (mPlayAnimtion)
	{
		ImGui::Begin(mAnimator.GetCurrentAnimation().name.c_str());
		ImGui::DragFloat("anim speed", &mAnimTimeScale, 0.001, 0.001f, 1.0f);
		ImGui::End();
		if ((float)mAnimTimer.GetMiliseceondsElapsed() / (1000.0f / mAnimTimeScale)  >= mAnimator.GetCurrentAnimation().duration)
			mAnimTimer.Restart();
		mAnimator.SetTimestamp((float)mAnimTimer.GetMiliseceondsElapsed() / (1000.0f / mAnimTimeScale));
		const AnimationChannel* channel = mAnimComp->GetCurrentChannel();
		if (channel)
		{
			//DirectX::XMMATRIX sample = mAnimComp->GetSample();
			//DirectX::XMVECTOR trans, quat, scale;
			//DirectX::XMMatrixDecompose(&scale, &quat, &trans, sample);
			//SetPosition(trans);
			//SetQuaternionRotation(quat);
			//float tempScale;
			//DirectX::XMStoreFloat(&tempScale, scale);
			//SetScale(tempScale, tempScale, tempScale);

			mAnimator.Bind(deviceContext);
		}
	}
	model.Draw(this->worldMatrix, viewProjectionMatrix);
	
}

void RenderableGameObject::UpdateMatrix()
{
	this->worldMatrix = XMMatrixScaling(this->scale.x, this->scale.y, this->scale.z) * XMMatrixRotationRollPitchYaw(this->rot.x, this->rot.y, this->rot.z) * XMMatrixTranslation(this->pos.x, this->pos.y, this->pos.z);
	this->UpdateDirectionVectors();
}
