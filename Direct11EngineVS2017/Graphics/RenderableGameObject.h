#pragma once
#include "GameObject3D.h"
#include "..\\Timer.h"

class RenderableGameObject : public GameObject3D
{
public:
	bool Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, 
		ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, ConstantBuffer<CB_PS_material>& cb_ps_material, IVertexShader * pVertexShader);
	bool InitAnimation(ConstantBuffer<CB_Bones>& cbufBones);

	void Draw(const XMMATRIX& viewProjectionMatrix);

protected:
	Model model;
	virtual void UpdateMatrix() override;

	XMMATRIX worldMatrix = XMMatrixIdentity();
private:
	ID3D11DeviceContext* deviceContext;

	Timer mAnimTimer;
	float mAnimTimeScale = 1.0f;

	bool mPlayAnimtion = false;
	MeshAnimator mAnimator;
	std::unique_ptr<AnimationComponent> mAnimComp;
};

