#pragma once
#include "AdapterReader.h"
#include "GameObject.h"
#include "Camera3D.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "..\\Timer.h"
#include "ImGui\\imgui.h"
#include "ImGui\\imgui_impl_win32.h"
#include "ImGui\\imgui_impl_dx11.h"
#include "RenderableGameObject.h"
#include "Light.h"
#include "TextureRender.h"

class ShaderMacro;
class Graphics
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	bool Initialize(HWND hwnd, int width, int height);
	void RenderShadowMap();
	void RenderFrame();
	void RenderImGui();
	void RenderText();
	void SetLight();
	void RenderBegin();
	void RenderEnd();

	ID3D11DeviceContext* GetDeviceContext();

	IVertexShader* CreateVertexShader(const std::string& filename, const std::vector<ShaderMacro>& macros);
	IVertexShader* GetVertexShader() const;
	void SetVertexShader(IVertexShader* pShader);
	IPixelShader* CreatePixelShader(const std::string& filename, const std::vector<ShaderMacro>& macros);
	IPixelShader* GetPixelShader() const;
	void SetPixelShader(IPixelShader* pShader);

	Camera3D camera3D;
	Light light;

	static Timer renderTimer;
private:
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool InitializeScene();

	// objs
	RenderableGameObject mainChara;
	RenderableGameObject platform;
	RenderableGameObject ball;

	ComPtr<ID3D11Device> device;
	ComPtr <ID3D11DeviceContext> deviceContext;
	ComPtr<IDXGISwapChain> swapchain;
	ComPtr<ID3D11RenderTargetView> renderTargetView;
	std::unique_ptr<CD3D11_VIEWPORT> viewport;
	// shaders
	D3DVertexShader* vertexshader;
	D3DPixelShader*  pixelshader;
	// c_buffers
	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	ConstantBuffer<CB_PS_light> cb_ps_light;
	ConstantBuffer<CB_PS_common> cb_ps_common;
	ConstantBuffer<CB_PS_material> cb_ps_material;
	ConstantBuffer<CB_Bones> cb_bones;
	ConstantBuffer<CB_PS_tonemapping_settings> cb_ps_tonemapping_settings;
	ConstantBuffer<CB_PS_brightExtract_settings> cb_ps_brightExtract_settings;
	ConstantBuffer<CB_PS_BlurSettings> cb_ps_blur_settings;
	ConstantBuffer<CB_PS_SkySettings> cb_ps_sky_settings;
	// shadow map
	ConstantBuffer<CB_PS_shadowmat> cb_ps_shadowmat;
	// depth stencil
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	ComPtr<ID3D11Texture2D> depthStencilBuffer;
	ComPtr<ID3D11DepthStencilState> depthStencilState;
	// RTT texture
	std::unique_ptr<TextureRender> hdr_RTT;
	std::unique_ptr<TextureRender> brightExtract_RTT;
	std::unique_ptr<TextureRender> gauss_blur_RTT;
	std::unique_ptr<TextureRender> bloom_RTT;
	// Deferred rendering
	std::unique_ptr<TextureRender> albedo_RTT;
	std::unique_ptr<TextureRender> position_RTT;
	std::unique_ptr<TextureRender> normals_RTT;
	std::unique_ptr<TextureRender> specColor_RTT;

	std::vector<ID3D11RenderTargetView*> RTVs;
	// rasterizer
	ComPtr<ID3D11RasterizerState> rasterrizerState;
	ComPtr<ID3D11RasterizerState> rasterrizerState_CullFront;
	// blendState
	ComPtr<ID3D11BlendState> blendState;
	// fontDraw
	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;
	// sampler
	ComPtr<ID3D11SamplerState> depthCmpSampler;
	ComPtr<ID3D11SamplerState> wrapSampler;
	ComPtr<ID3D11SamplerState> clapSampler;
	// textures
	ComPtr<ID3D11ShaderResourceView> toneTexture;
	// pipelines
	std::unique_ptr<IPipeline> Pipeline_ShadowMap;
	std::unique_ptr<IPipeline> Pipeline_General3D;
	std::unique_ptr<IPipeline> Pipeline_Nolight3D;


	bool enableCelshading = false;
	bool enableProcSky = true;

	int window_width = 0;
	int window_height = 0;

	Timer fpsTimer;
	Timer deltaTimer;
	float deltaTime = 0;
	float launchTime = 0;

	int gaussBlurPasses = 5;
	float exposure = 0.85f;
	float brightThreshold = 0.92f;
};