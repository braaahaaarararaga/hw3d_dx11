#include "Graphics.h"

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	window_height = height;
	window_width = width;
	fpsTimer.Start();
	deltaTimer.Start();

	if (!InitializeDirectX(hwnd))
		return false;

	if (!InitializeShaders())
		return false;

	if (!InitializeScene())
		return false;
	deviceContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device.Get(), deviceContext.Get());
	ImGui::StyleColorsDark();

	return true;
}

void Graphics::RenderShadowMap() // TODO: abstract shadow pipeline
{	
	light.SetUpShadowMap();
	{
		mainChara.Draw(light.GetVpMatrix(), Pipeline_ShadowMap.get());
		platform.Draw(light.GetVpMatrix(), Pipeline_ShadowMap.get());
		ball.Draw(light.GetVpMatrix(), Pipeline_ShadowMap.get());
	}
	cb_ps_shadowmat.data.shadowMatrix = XMMatrixTranspose(light.GetVpMatrix());
	cb_ps_shadowmat.ApplyChanges();
	deviceContext->PSSetConstantBuffers(3, 1, cb_ps_shadowmat.GetAddressOf());
}

void Graphics::RenderFrame()
{
	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
	//Set the Viewport
	deviceContext->RSSetViewports(1, viewport.get());
	deviceContext->PSSetShaderResources(5, 1, toneTexture.GetAddressOf());
	
	// Deferred Rendering PASS
	{

		IPixelShader*  ps;
		ps = ResourceManager::GetPixelShader("Graphics/Shaders/PS_General3D_DeferredShading.hlsl", this);
		SetPixelShader(ps);
		TextureRender::SetMultiRenderTarget(deviceContext.Get(), RTVs, depthStencilView.Get(), viewport.get());

		XMMATRIX camVPMat = camera3D.GetViewMatrix() * camera3D.GetProjectionMatrix();
		platform.Draw(camVPMat, Pipeline_General3D.get());
		mainChara.Draw(camVPMat, Pipeline_General3D.get());
		ball.Draw(camVPMat, Pipeline_General3D.get());

		light.Draw(camVPMat, Pipeline_Nolight3D.get());


		deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);
		deviceContext->RSSetViewports(1, viewport.get());
	}


	light.BindShadowResourceView();
	// HDR PASS
	hdr_RTT->Begin(deviceContext.Get());
	{	// TODO: refactory render pipelines & render pass
		//// Draw Models
		//IPixelShader*  ps;
		//ps = ResourceManager::GetPixelShader("Graphics/Shaders/PS_General3D.hlsl", this);
		//SetPixelShader(ps);

		//XMMATRIX camVPMat = camera3D.GetViewMatrix() * camera3D.GetProjectionMatrix();
		//platform.Draw(camVPMat, Pipeline_General3D.get());
		//if (enableCelshading)
		//{
		//	ps = ResourceManager::GetPixelShader("Graphics/Shaders/PS_CelShading.hlsl", this);
		//	SetPixelShader(ps);
		//}
		//mainChara.Draw(camVPMat, Pipeline_General3D.get());
		//ball.Draw(camVPMat, Pipeline_General3D.get());

		//light.Draw(camVPMat, Pipeline_Nolight3D.get());

		// Deferred shading
		{
			IVertexShader* sst = ResourceManager::GetVertexShader("Graphics/Shaders/VS_ScreenSizeTri.hlsl", this);
			SetVertexShader(sst);
			IPixelShader*  ps = ResourceManager::GetPixelShader("Graphics/Shaders/PS_DeferredShading_PP.hlsl", this);
			if (enableCelshading)
			{
				ps = ResourceManager::GetPixelShader("Graphics/Shaders/PS_CelShading_Deferred.hlsl", this);
			}
			SetPixelShader(ps);
			deviceContext->PSSetShaderResources(0, 1, position_RTT->GetOutputTexture());
			deviceContext->PSSetShaderResources(1, 1, normals_RTT->GetOutputTexture());
			deviceContext->PSSetShaderResources(2, 1, albedo_RTT->GetOutputTexture());
			deviceContext->PSSetShaderResources(3, 1, specColor_RTT->GetOutputTexture());
			deviceContext->Draw(3, 0);
		}
		if (enableProcSky)
		{
			ID3D11RenderTargetView* rtv = hdr_RTT->GetRenderTargetView();
			deviceContext->OMSetRenderTargets(1, &rtv, depthStencilView.Get());
			IVertexShader* dynamic_sky = ResourceManager::GetVertexShader("Graphics/Shaders/VS_DynamicSky.hlsl", this);
			SetVertexShader(dynamic_sky);
			IPixelShader*  dSkyPS = ResourceManager::GetPixelShader("Graphics/Shaders/PS_DynamicSky.hlsl", this);
			SetPixelShader(dSkyPS);

			cb_ps_sky_settings.ApplyChanges();
			deviceContext->PSSetConstantBuffers(4, 1, cb_ps_sky_settings.GetAddressOf());

			deviceContext->Draw(3, 0);
		}
	}
	hdr_RTT->End(deviceContext.Get());
	//


	// PostProcess //
	// BLOOM PASS
		// EXTRACT BRIGHT
	{
		brightExtract_RTT->Begin(deviceContext.Get());
		IVertexShader* sst = ResourceManager::GetVertexShader("Graphics/Shaders/VS_ScreenSizeTri.hlsl", this);
		SetVertexShader(sst);
		IPixelShader*  ps = ResourceManager::GetPixelShader("Graphics/Shaders/PS_BrightExtract.hlsl", this);
		SetPixelShader(ps);

		deviceContext->PSSetShaderResources(0, 1, hdr_RTT->GetOutputTexture());
		cb_ps_brightExtract_settings.ApplyChanges();
		deviceContext->PSSetConstantBuffers(4, 1, cb_ps_brightExtract_settings.GetAddressOf());

		deviceContext->Draw(3, 0);
		brightExtract_RTT->End(deviceContext.Get());
	}
		// GAUSS BLUR 
	{
		IVertexShader* sst = ResourceManager::GetVertexShader("Graphics/Shaders/VS_ScreenSizeTri.hlsl", this);
		SetVertexShader(sst);
		IPixelShader*  ps = ResourceManager::GetPixelShader("Graphics/Shaders/PS_GaussBlur.hlsl", this);
		SetPixelShader(ps);

		deviceContext->PSSetConstantBuffers(4, 1, cb_ps_blur_settings.GetAddressOf());

		ID3D11RenderTargetView* shadowRenderTargetView[1] = { 0 };
		for (int i = 0; i < gaussBlurPasses * 2; i++)
		{
			deviceContext->OMSetRenderTargets(1, shadowRenderTargetView, depthStencilView.Get());
			// Unbind whatever is currently bound, to avoid binding on input/output at same time
			if (i % 2 == 0)
			{
				gauss_blur_RTT->Begin(deviceContext.Get());

				deviceContext->PSSetShaderResources(0, 1, brightExtract_RTT->GetOutputTexture());
				cb_ps_blur_settings.data.Horizontal = true;
				cb_ps_blur_settings.ApplyChanges();
				deviceContext->Draw(3, 0);
				gauss_blur_RTT->End(deviceContext.Get());
			}
			else
			{
				brightExtract_RTT->Begin(deviceContext.Get());

				deviceContext->PSSetShaderResources(0, 1, gauss_blur_RTT->GetOutputTexture());
				cb_ps_blur_settings.data.Horizontal = false;
				cb_ps_blur_settings.ApplyChanges();
				deviceContext->Draw(3, 0);
				brightExtract_RTT->End(deviceContext.Get());
			}
		}

		deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
	}
		// BLOOM
	{
		bloom_RTT->Begin(deviceContext.Get());

		IVertexShader* sst = ResourceManager::GetVertexShader("Graphics/Shaders/VS_ScreenSizeTri.hlsl", this);
		SetVertexShader(sst);
		IPixelShader*  ps = ResourceManager::GetPixelShader("Graphics/Shaders/PS_Bloom.hlsl", this);
		SetPixelShader(ps);

		deviceContext->PSSetShaderResources(0, 1, hdr_RTT->GetOutputTexture());
		deviceContext->PSSetShaderResources(1, 1, brightExtract_RTT->GetOutputTexture());

		deviceContext->Draw(3, 0);
		bloom_RTT->End(deviceContext.Get());

	}
	// TONE_MAPPING PASS
	{
		IVertexShader* sst = ResourceManager::GetVertexShader("Graphics/Shaders/VS_ScreenSizeTri.hlsl", this);
		SetVertexShader(sst);
		IPixelShader*  ps = ResourceManager::GetPixelShader("Graphics/Shaders/PS_ToneMapping.hlsl", this);
		SetPixelShader(ps);

		deviceContext->PSSetShaderResources(0, 1, bloom_RTT->GetOutputTexture());
		cb_ps_tonemapping_settings.ApplyChanges();
		deviceContext->PSSetConstantBuffers(4, 1, cb_ps_tonemapping_settings.GetAddressOf());
		deviceContext->Draw(3, 0);
	}
	//
}

void Graphics::RenderImGui()
{
	// Create imGui Test Window
	ImGui::Begin("Light Controls");
	ImGui::ColorEdit3("Ambient Light", &this->cb_ps_light.data.ambientLightColor.x);
	//ImGui::DragFloat("Ambient Strength", &this->cb_ps_light.data.ambientLightStrength, 0.001, 0.0f, 1.0f);
	float t = cos(launchTime) * 0.5f + 0.5f;
	cb_ps_light.data.ambientLightStrength = t * 0.25f;
	ImGui::NewLine();
	if (ImGui::ColorEdit3("Dynamic Light Color", &this->light.lightColor.x))
	{
		Color color = Color((BYTE)(light.lightColor.x * 255), (BYTE)(light.lightColor.y * 255), (BYTE)(light.lightColor.z * 255), (BYTE)0);
		light.SetMeshDiffuseColor(color);
	}
	ImGui::DragFloat("Dynamic Light Strength", &this->light.lightStrenght, 0.05, 0.0f, 20.0f);
	ImGui::End();


	ImGui::Begin(mainChara.GetAnimator().GetCurrentAnimation().name.c_str());
	ImGui::DragFloat("anim speed", &mainChara.GetAnimaTimeScale(), 0.001, 0.001f, 1.0f);
	ImGui::End();

	ImGui::Begin("bloom pass review");
	ImGui::Image((void*)*brightExtract_RTT->GetOutputTexture(), ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowWidth()));
	ImGui::End();

	ImGui::Begin("deferred shading pass review");
	ImGui::Text("position");
	ImGui::Image((void*)*position_RTT->GetOutputTexture(), ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowWidth()));
	ImGui::Text("normal");
	ImGui::Image((void*)*normals_RTT->GetOutputTexture(), ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowWidth()));
	ImGui::Text("albedo");
	ImGui::Image((void*)*albedo_RTT->GetOutputTexture(), ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowWidth()));
	ImGui::Text("specular");
	ImGui::Image((void*)*specColor_RTT->GetOutputTexture(), ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowWidth()));
	ImGui::End();

	ImGui::Begin("Shader Settings");
	ImGui::Checkbox("CelShading", &enableCelshading);
	ImGui::DragFloat("Exposure", &exposure, 0.01f, 0.5f, 10.0f);
	cb_ps_tonemapping_settings.data.exposure = exposure + (1 - pow(t, 2)) * 0.8f;
	ImGui::DragFloat("BrightThreshold", &brightThreshold, 0.01f, 0.01f, 10.0f);
	cb_ps_brightExtract_settings.data.brightThreshold = brightThreshold + t * 1;
	ImGui::DragInt("GaussBlurPasses", &gaussBlurPasses, 0.1, 0, 8);
	ImGui::End();

	ImGui::Begin("ProceduralSky Settings");
	ImGui::Checkbox("Enable", &enableProcSky);
	ImGui::Checkbox("Enable Cloud", reinterpret_cast<bool*>(&cb_ps_sky_settings.data.EnableCloud));
	ImGui::DragFloat("Cloudiness", &cb_ps_sky_settings.data.Cloudiness, 0.001f, 0.001f, 1.0f);
	ImGui::DragFloat("CloudScale", &cb_ps_sky_settings.data.CloudScale, 0.001f, 0.001f, 1.0f);
	ImGui::DragFloat("CloudSpeed", &cb_ps_sky_settings.data.CloudSpeed, 0.01f, 0.03f, 2.0f);
	ImGui::End();

	// Assemble Together Draw Data
	ImGui::Render();
	// Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Graphics::RenderText()
{
	// Draw text
	static int fpsCounter = 0;
	static std::wstring fpsString = L"FPS: 0";
	fpsCounter++;
	if (fpsTimer.GetMiliseceondsElapsed() > 1000.0)
	{
		fpsString = L"FPS: " + std::to_wstring(fpsCounter);
		fpsCounter = 0;
		fpsTimer.Restart();
	}
	static std::wstring controlGuide = L"Controls\n Move: W/S/A/D/Space/Ctrl/Shift\n Roll: Hold mouse right button\n Set light pos: C";
	spriteBatch->Begin();
	spriteFont->DrawString(spriteBatch.get(), fpsString.c_str(), DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f,
		DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(0.65f, 0.65f));
	spriteFont->DrawString(spriteBatch.get(), controlGuide.c_str(), DirectX::XMFLOAT2(0, 16), DirectX::Colors::White, 0.0f,
		DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
	spriteBatch->End();
}

void Graphics::SetLight()
{
	cb_ps_common.data.resolution.x = window_width;
	cb_ps_common.data.resolution.y = window_height;
	cb_ps_common.data.time = launchTime;
	cb_ps_common.data.deltaTime = deltaTime;
	cb_ps_common.data.eyePos = camera3D.GetPositionFloat3();
	cb_ps_common.data.cameraInvVP = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(NULL, camera3D.GetViewMatrix() * camera3D.GetProjectionMatrix()));
	deviceContext->PSSetConstantBuffers(1, 1, cb_ps_common.GetAddressOf());
	cb_ps_common.ApplyChanges();

	cb_ps_light.data.dynamicLightColor = light.lightColor;
	cb_ps_light.data.dynamicLightStrength = light.lightStrenght;
	cb_ps_light.data.dynamicPosition = light.GetPositionFloat3();
	cb_ps_light.data.dynamicLightAttenuation_a = light.attenuation_a;
	cb_ps_light.data.dynamicLightAttenuation_b = light.attenuation_b;
	cb_ps_light.data.dynamicLightAttenuation_c = light.attenuation_c;
	deviceContext->PSSetConstantBuffers(0, 1, cb_ps_light.GetAddressOf());
	cb_ps_light.ApplyChanges();
}

void Graphics::RenderBegin()
{
	// graphic pipeline
	// Input-Assembler--IA
	// Vertex Shader---VS
	// Hull Shader
	// Tessellates-----subdivision surfaces
	// Domain Shader
	// Geometry Shader-GS
	//   |- Stream Output---- output data to buffer
	// Rasterizer------Rasterization vector information to raster image
	// Pixel Shader----PS
	// Output-Merger---OM

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	deltaTime = deltaTimer.GetTimeElapsed();
	deltaTimer.Restart();
	launchTime += deltaTime;

	float bgcolor[] = { .0f, .0f, .0f, 1.0f };
	deviceContext->ClearRenderTargetView(renderTargetView.Get(), bgcolor);
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	deviceContext->PSSetSamplers(0, 1, this->depthCmpSampler.GetAddressOf());

	deviceContext->RSSetState(rasterrizerState.Get());
	deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);
	deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);

}

void Graphics::RenderEnd()
{
	swapchain->Present(0u, NULL);
}

ID3D11DeviceContext * Graphics::GetDeviceContext()
{
	return deviceContext.Get();
}

IVertexShader * Graphics::CreateVertexShader(const std::string & filename, const std::vector<ShaderMacro>& macros)
{
	return new D3DVertexShader(device.Get(), filename, macros);
}

IVertexShader * Graphics::GetVertexShader() const
{
	return vertexshader;
}

IPixelShader * Graphics::CreatePixelShader(const std::string & filename, const std::vector<ShaderMacro>& macros)
{
	return new D3DPixelShader(device.Get(), filename, macros);
}

IPixelShader * Graphics::GetPixelShader() const
{
	return pixelshader;
}

void Graphics::SetPixelShader(IPixelShader * pShader)
{
	pixelshader = static_cast<D3DPixelShader*>(pShader);
	deviceContext->PSSetShader(pixelshader ? pixelshader->GetShader(device.Get()) : nullptr, nullptr, 0);
}

void Graphics::SetVertexShader(IVertexShader * pShader)
{
	vertexshader = static_cast<D3DVertexShader*>(pShader);
	deviceContext->VSSetShader(vertexshader ? vertexshader->GetShader(device.Get()) : nullptr, nullptr, 0);
	deviceContext->IASetInputLayout(vertexshader ? vertexshader->GetLayout() : nullptr);
}

bool Graphics::InitializeDirectX(HWND hwnd)
{
	std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

	if (adapters.size() < 1)
	{
		ErrorLogger::Log("NO DXGI Adapters found.");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC scd = { 0 };

	scd.BufferDesc.Width = window_width;
	scd.BufferDesc.Height = window_height;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = hwnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(adapters[0].pAdapter, // IDXGI Adapter
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL, // FOR SOFTWARE DRIVER TYPE
		NULL, // FLAGS FOR RUNTIME LAYERS
		NULL, // FEATURE LEVELS ARRAY
		0,	  // # OF FEATURE LEVELS IN ARRAY
		D3D11_SDK_VERSION,
		&scd, // Swapchain description
		swapchain.GetAddressOf(), // Swapchain Address
		device.GetAddressOf(),	// Device Address
		NULL, // SUPPROTED FEATURE LEVEL
		deviceContext.GetAddressOf()
	);

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create device and swapchain");
		return false;
	}
	COM_ERROR_IF_FAILED(hr, "Failed to create device and swapchain.");


	ComPtr<ID3D11Texture2D> backBuffer;
	hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	
	COM_ERROR_IF_FAILED(hr, "Failed to get swapchain buffer.");

	hr = device->CreateRenderTargetView(backBuffer.Get(), NULL, this->renderTargetView.GetAddressOf());

	COM_ERROR_IF_FAILED(hr, "Failed to create render target view.");

	//Describe our Depth/Stencil Buffer
	CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, window_width, window_height);
	//D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = device->CreateTexture2D(&depthStencilDesc, NULL, depthStencilBuffer.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil buffer.");


	hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), NULL, depthStencilView.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil view.");



	// Create depth stencil state
	CD3D11_DEPTH_STENCIL_DESC depthstencildesc(D3D11_DEFAULT);
	//D3D11_DEPTH_STENCIL_DESC depthstencildesc;
	depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

	hr = device->CreateDepthStencilState(&depthstencildesc, depthStencilState.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil desc.");


	//Create the Viewport
	viewport = std::make_unique<CD3D11_VIEWPORT>(0.0f, 0.0f, static_cast<float>(window_width), static_cast<float>(window_height));

	
	// shadow sampler

	CD3D11_SAMPLER_DESC samplerDesc(D3D11_DEFAULT);
	// compare sampler
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

	hr = device->CreateSamplerState(&samplerDesc, depthCmpSampler.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create sampler state.");
	deviceContext->PSSetSamplers(0, 1, this->depthCmpSampler.GetAddressOf());

	// Create Rasterizer State
	CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
	hr = device->CreateRasterizerState(&rasterizerDesc, rasterrizerState.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");

	// Create Rasterizer State for culling front
	CD3D11_RASTERIZER_DESC rasterizerDesc_CullFront(D3D11_DEFAULT);
	rasterizerDesc_CullFront.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
	hr = device->CreateRasterizerState(&rasterizerDesc_CullFront, rasterrizerState_CullFront.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");

	// Create Blend State
	D3D11_RENDER_TARGET_BLEND_DESC rtbd = { 0 };
	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_BLEND_DESC blendDesc = { 0 };
	blendDesc.RenderTarget[0] = rtbd;

	hr = device->CreateBlendState(&blendDesc, blendState.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create blendstate.");
	
	spriteBatch = std::make_unique<DirectX::SpriteBatch>(deviceContext.Get());
	spriteFont = std::make_unique<DirectX::SpriteFont>(device.Get(), L"Data\\Fonts\\comic_sans_ms_16.spritefont");

	//Create sampler description for sampler state
	//CD3D11_SAMPLER_DESC samplerDesc(D3D11_DEFAULT);
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

	hr = device->CreateSamplerState(&samplerDesc, wrapSampler.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create sampler state.");
	deviceContext->PSSetSamplers(1, 1, wrapSampler.GetAddressOf());
	
	samplerDesc.AddressU  = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV  = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW  = D3D11_TEXTURE_ADDRESS_CLAMP;

	hr = device->CreateSamplerState(&samplerDesc, clapSampler.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create sampler state.");
	deviceContext->PSSetSamplers(2, 1, clapSampler.GetAddressOf());



	return true;
}

bool Graphics::InitializeShaders()
{
	Pipeline_ShadowMap = std::make_unique<ShadowMapPipeline>();
	Pipeline_General3D = std::make_unique<General3DPipeline>();
	Pipeline_Nolight3D = std::make_unique<NoLight3DPipeline>();

	hdr_RTT = std::make_unique<TextureRender>(device.Get(), window_width, window_height, TexFormat::TEX_FORMAT_R16G16B16A16_FLOAT, false);
	brightExtract_RTT = std::make_unique<TextureRender>(device.Get(), window_width / 2, window_height / 2, TexFormat::TEX_FORMAT_R16G16B16A16_FLOAT, false);
	gauss_blur_RTT = std::make_unique<TextureRender>(device.Get(), window_width / 2, window_height / 2, TexFormat::TEX_FORMAT_R16G16B16A16_FLOAT, false);
	bloom_RTT = std::make_unique<TextureRender>(device.Get(), window_width, window_height, TexFormat::TEX_FORMAT_R16G16B16A16_FLOAT, false);

	albedo_RTT = std::make_unique<TextureRender>(device.Get(), window_width, window_height, TexFormat::TEX_FORMAT_R8G8B8A8_UNORM, false);
	position_RTT = std::make_unique<TextureRender>(device.Get(), window_width, window_height, TexFormat::TEX_FORMAT_R16G16B16A16_FLOAT, false);
	normals_RTT = std::make_unique<TextureRender>(device.Get(), window_width, window_height, TexFormat::TEX_FORMAT_R16G16B16A16_FLOAT, false);
	specColor_RTT = std::make_unique<TextureRender>(device.Get(), window_width, window_height, TexFormat::TEX_FORMAT_R8G8B8A8_UNORM, false);

	RTVs.push_back(position_RTT->GetRenderTargetView());
	RTVs.push_back(normals_RTT->GetRenderTargetView());
	RTVs.push_back(albedo_RTT->GetRenderTargetView());
	RTVs.push_back(specColor_RTT->GetRenderTargetView());
	return true;
}

bool Graphics::InitializeScene()
{
	HRESULT hr;
		// load texture
	hr = DirectX::CreateWICTextureFromFile(device.Get(), L"Data\\Textures\\tone_2.png", nullptr, toneTexture.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create wic texture.");

	//initialize constant buffer
	hr = this->cb_vs_vertexshader.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = this->cb_ps_light.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");
	cb_ps_light.data.ambientLightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
	cb_ps_light.data.ambientLightStrength = 0.2f;

	hr = this->cb_ps_shadowmat.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = this->cb_ps_common.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = this->cb_ps_material.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = this->cb_bones.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = this->cb_ps_tonemapping_settings.Initialize(device.Get(), deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");
	cb_ps_tonemapping_settings.data.exposure = 1.4f;
	
	hr = this->cb_ps_brightExtract_settings.Initialize(device.Get(), deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");
	cb_ps_brightExtract_settings.data.brightThreshold = 1.0f;

	hr = this->cb_ps_blur_settings.Initialize(device.Get(), deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = this->cb_ps_sky_settings.Initialize(device.Get(), deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");
	cb_ps_sky_settings.data.Cloudiness = 0.2f;
	cb_ps_sky_settings.data.CloudScale = 0.03f;
	cb_ps_sky_settings.data.CloudSpeed = 0.1f;
	cb_ps_sky_settings.data.EnableCloud = 1;


#ifdef _DEBUG
	cb_ps_light.SetDebugName("cb_ps_light");
	cb_ps_material.SetDebugName("ps_material");
	cb_vs_vertexshader.SetDebugName("cb_vs_world");
	cb_ps_common.SetDebugName("cb_ps_common");
	cb_bones.SetDebugName("vs_bone_transforms"); 
	cb_ps_tonemapping_settings.SetDebugName("cb_ps_tonemapping_settings");
	cb_ps_blur_settings.SetDebugName("cb_ps_blur_setting_buffer");
	cb_ps_sky_settings.SetDebugName("cb_ps_sky_setting_buffer");
#endif
	// Initialize Model(s)

	if (!mainChara.Initialize("Data\\Objects\\akai_e_espiritu@Taunt.fbx", this->device.Get(), this->deviceContext.Get(),
		cb_vs_vertexshader, cb_ps_material, this))
	{
		COM_ERROR_IF_FAILED(-1, "Failed to load model file.");
		return false;
	}
	mainChara.SetScale(0.07f, 0.07f, 0.07f);
	mainChara.AdjustPosition(0.0f, 0.0f, 3.0f);
	mainChara.InitAnimation(cb_bones);

	if (!platform.Initialize("Data\\Objects\\brick_wall\\brick_wall.obj", this->device.Get(), this->deviceContext.Get(), cb_vs_vertexshader, cb_ps_material,this))
	{
		COM_ERROR_IF_FAILED(-1, "Failed to load model file.");
		return false;
	}
	platform.AdjustRotation(DirectX::XMConvertToRadians(90.0f), 0.0f, 0.0f);
	platform.SetScale(20.0f, 20.0f);

	if (!ball.Initialize("Data\\Objects\\sphere_smooth.obj", this->device.Get(), this->deviceContext.Get(), cb_vs_vertexshader, cb_ps_material, this))
	{
		COM_ERROR_IF_FAILED(-1, "Failed to load model file.");
		return false;
	}
	ball.AdjustPosition(3.0f, 3.0f, 0.0f);

	if (!light.Initialize(this->device.Get(), this->deviceContext.Get(), cb_vs_vertexshader, cb_ps_material, this))
	{
		COM_ERROR_IF_FAILED(-1, "Failed to load model file.");
		return false;
	}
	light.AdjustPosition(0.0f, 10.0f, -6.0f);
	light.AdjustRotation(DirectX::XMConvertToRadians(20.0f), 0.0f, 0.0f);

	camera3D.SetPosition(0.0f, 5.0f, -2.0f);
	camera3D.SetProjectionValues(90.0f, static_cast<float>(window_width) / static_cast<float>(window_height), 0.1f, 1000.0f);
	
	return true;
}
