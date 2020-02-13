#include "Graphics.h"
#include "RenderableGameObject.h"
#include "ImGui\\imgui.h"

bool RenderableGameObject::Initialize(const std::string & filePath, ID3D11Device * device, ID3D11DeviceContext * deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, ConstantBuffer<CB_PS_material>& cb_ps_material, Graphics * gfx)
{
	this->deviceContext = deviceContext;
	if (!model.Initialize(filePath, device, deviceContext, cb_vs_vertexshader, cb_ps_material, gfx))
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

void RenderableGameObject::Draw(const XMMATRIX & viewProjectionMatrix, IPipeline* pipeline)
{
	if (mPlayAnimtion)
	{
		float time = (float)mAnimTimer.GetMiliseceondsElapsed() / 1000.0f;
		if (time * mAnimTimeScale  >= mAnimator.GetCurrentAnimation().duration)
			mAnimTimer.Restart();
		mAnimator.SetTimestamp(time * mAnimTimeScale);
		const AnimationChannel* channel = mAnimComp->GetCurrentChannel();
		if (channel)
		{
			mAnimator.Bind(deviceContext);
		}
	}
	model.Draw(this->worldMatrix, viewProjectionMatrix, pipeline);
	
}

float & RenderableGameObject::GetAnimaTimeScale()
{
	return mAnimTimeScale;
}

MeshAnimator & RenderableGameObject::GetAnimator()
{
	return mAnimator;
}

void RenderableGameObject::SetMeshDiffuseColor(Color & color)
{
	model.SetMeshDiffuseColor(color);
}

void RenderableGameObject::UpdateMatrix()
{
	this->worldMatrix = XMMatrixScaling(this->scale.x, this->scale.y, this->scale.z) * XMMatrixRotationRollPitchYaw(this->rot.x, this->rot.y, this->rot.z) * XMMatrixTranslation(this->pos.x, this->pos.y, this->pos.z);
	this->UpdateDirectionVectors();
}
