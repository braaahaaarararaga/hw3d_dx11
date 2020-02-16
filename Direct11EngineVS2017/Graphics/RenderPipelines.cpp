#include "Graphics.h"
#include "RenderPipelines.h"

void ShadowMapPipeline::Render(Graphics* gfx, Mesh& mesh)
{
	auto macros = ResourceManager::BuildMacrosForMesh(mesh);

	IVertexShader* vertexshader = ResourceManager::GetVertexShader("Graphics/Shaders/VS_ShadowMapping.hlsl", gfx, macros);
	gfx->SetVertexShader(vertexshader);

	mesh.Bind(gfx);
	mesh.BindMaterial();
	gfx->GetDeviceContext()->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	gfx->GetDeviceContext()->DrawIndexed(mesh.GetIndexBuffer().IndexCount(), 0, 0);
}

void General3DPipeline::Render(Graphics* gfx, Mesh& mesh)
{
	auto macros = ResourceManager::BuildMacrosForMesh(mesh);

	IVertexShader* vertexshader = ResourceManager::GetVertexShader("Graphics/Shaders/VS_General3D.hlsl", gfx, macros);
	gfx->SetVertexShader(vertexshader);

	mesh.Bind(gfx);
	mesh.BindMaterial();
	gfx->GetDeviceContext()->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	gfx->GetDeviceContext()->DrawIndexed(mesh.GetIndexBuffer().IndexCount(), 0, 0);
}

void NoLight3DPipeline::Render(Graphics * gfx, Mesh & mesh)
{
	auto macros = ResourceManager::BuildMacrosForMesh(mesh);

	IVertexShader* vertexshader = ResourceManager::GetVertexShader("Graphics/Shaders/VS_nolight.hlsl", gfx, macros);
	gfx->SetVertexShader(vertexshader);
	IPixelShader*  ps = ResourceManager::GetPixelShader("Graphics/Shaders/PS_nolight.hlsl", gfx);
	gfx->SetPixelShader(ps);

	mesh.Bind(gfx);
	mesh.BindMaterial();
	gfx->GetDeviceContext()->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	gfx->GetDeviceContext()->DrawIndexed(mesh.GetIndexBuffer().IndexCount(), 0, 0);
}