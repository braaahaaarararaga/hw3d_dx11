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
	shadowMapRTT->SetUpToRender(deviceContext.Get());

	XMMATRIX lightViewMat = XMMatrixLookAtLH(light.GetPositionVector(), light.GetPositionVector() + light.GetForwardVector(), { 0.0f, 1.0f, 0.0f });
	float aspect = (float)shadow_width / (float)shadow_height;
	XMMATRIX lightProjMat = XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f), aspect, 0.1f, 800.0f);

	deviceContext->PSSetConstantBuffers(3, 1, cb_ps_shadowmat.GetAddressOf());
	cb_ps_shadowmat.data.shadowMatrix = XMMatrixTranspose(lightViewMat * lightProjMat);
	cb_ps_shadowmat.ApplyChanges();
	{
		deviceContext->VSSetShader(d3dvertexshader_shadowmap_anim.get()->GetShader(device.Get()), NULL, 0);
		deviceContext->IASetInputLayout(d3dvertexshader_shadowmap_anim.get()->GetLayout());
		gameObj.Draw(lightViewMat * lightProjMat);
	
	
		deviceContext->VSSetShader(d3dvertexshader_shadowmap.get()->GetShader(device.Get()), NULL, 0);
		deviceContext->IASetInputLayout(d3dvertexshader_shadowmap.get()->GetLayout());
		gameObj2.Draw(lightViewMat * lightProjMat);
		gameObj3.Draw(lightViewMat * lightProjMat);
	}
}

void Graphics::RenderFrame()
{
	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
	//Set the Viewport
	deviceContext->RSSetViewports(1, viewport.get());
	deviceContext->PSSetShaderResources(5, 1, toneTexture.GetAddressOf());
	
	shadowMapRTT->BindTexture(deviceContext.Get(), 4);

	ImGui::Begin("Shader Settings");
	ImGui::Checkbox("Tone Shading", &enableToneshading);
	ImGui::NewLine();
	ImGui::End();

	{	// TODO: refactory render pipelines
		if(enableToneshading)
			deviceContext->PSSetShader(pixelshader_tonemapping.GetShader(), NULL, 0);
		else
			deviceContext->PSSetShader(pixelshader.GetShader(), NULL, 0);
		deviceContext->VSSetShader(d3dvertexshader_animation.get()->GetShader(device.Get()), NULL, 0);
		deviceContext->IASetInputLayout(d3dvertexshader_animation.get()->GetLayout());
		gameObj.Draw(Camera3D.GetViewMatrix() * Camera3D.GetProjectionMatrix());
	}
	{
		deviceContext->PSSetShader(pixelshader.GetShader(), NULL, 0);
		deviceContext->VSSetShader(d3dvertexshader.get()->GetShader(device.Get()), NULL, 0);
		deviceContext->IASetInputLayout(d3dvertexshader.get()->GetLayout());
		gameObj2.Draw(Camera3D.GetViewMatrix() * Camera3D.GetProjectionMatrix());

		if (enableToneshading)
			deviceContext->PSSetShader(pixelshader_tonemapping.GetShader(), NULL, 0);
		else
			deviceContext->PSSetShader(pixelshader.GetShader(), NULL, 0);

		gameObj3.Draw(Camera3D.GetViewMatrix() * Camera3D.GetProjectionMatrix());
	}
	{
		deviceContext->PSSetShader(pixelshader_nolight.GetShader(), NULL, 0);
		deviceContext->VSSetShader(d3dvertexshader_nolight.get()->GetShader(device.Get()), NULL, 0);
		deviceContext->IASetInputLayout(d3dvertexshader_nolight.get()->GetLayout());
		light.Draw(Camera3D.GetViewMatrix() * Camera3D.GetProjectionMatrix());
	}
	{
		deviceContext->PSSetShader(pixelshader_dynamic_sky.GetShader(), NULL, 0);
		deviceContext->VSSetShader(d3dvertexshader_dynamic_sky.get()->GetShader(device.Get()), NULL, 0);
		deviceContext->IASetInputLayout(d3dvertexshader_dynamic_sky.get()->GetLayout());
		deviceContext->Draw(3, 0);
	}

}

void Graphics::RenderImGui()
{
	// Create imGui Test Window
	ImGui::Begin("Light Controls");
	ImGui::ColorEdit3("Ambient Light", &this->cb_ps_light.data.ambientLightColor.x);
	ImGui::DragFloat("Ambient Strength", &this->cb_ps_light.data.ambientLightStrength, 0.001, 0.0f, 1.0f);
	ImGui::NewLine();
	ImGui::ColorEdit3("Dynamic Light Color", &this->light.lightColor.x);
	ImGui::DragFloat("Dynamic Light Strength", &this->light.lightStrenght, 0.01, 0.0f, 10.0f);
	ImGui::DragFloat("Dynamic Light Attenuation A", &this->light.attenuation_a, 0.001, 0.1f, 10.0f);
	ImGui::DragFloat("Dynamic Light Attenuation B", &this->light.attenuation_b, 0.001, 0.0f, 10.0f);
	ImGui::DragFloat("Dynamic Light Attenuation C", &this->light.attenuation_c, 0.001, 0.0f, 10.0f);
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
	cb_ps_common.data.eyePos = Camera3D.GetPositionFloat3();
	cb_ps_common.data.cameraInvVP = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(NULL, Camera3D.GetViewMatrix() * Camera3D.GetProjectionMatrix()));
	cb_ps_common.ApplyChanges();
	deviceContext->PSSetConstantBuffers(1, 1, cb_ps_common.GetAddressOf());

	cb_ps_light.data.dynamicLightColor = light.lightColor;
	cb_ps_light.data.dynamicLightStrength = light.lightStrenght;
	cb_ps_light.data.dynamicPosition = light.GetPositionFloat3();
	cb_ps_light.data.dynamicLightAttenuation_a = light.attenuation_a;
	cb_ps_light.data.dynamicLightAttenuation_b = light.attenuation_b;
	cb_ps_light.data.dynamicLightAttenuation_c = light.attenuation_c;

	cb_ps_light.ApplyChanges();
	deviceContext->PSSetConstantBuffers(0, 1, cb_ps_light.GetAddressOf());
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

	deviceContext->RSSetState(rasterrizerState.Get());
	deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);
	deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);

}

void Graphics::RenderEnd()
{
	swapchain->Present(0u, NULL);
}

IVertexShader * Graphics::CreateVertexShader(const std::string & filename)
{
	return new D3DVertexShader(device.Get(), filename);
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
	deviceContext->PSSetSamplers(1, 1, this->depthCmpSampler.GetAddressOf());

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

	hr = device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create sampler state.");
	deviceContext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());
	
	shadowMapRTT = std::make_unique<ShadowMapRTT>(device.Get(), shadow_width, shadow_height);
	

	return true;
}

bool Graphics::InitializeShaders()
{
	std::wstring shaderfolder = L"";
#pragma region DetermineShaderPath
	if (IsDebuggerPresent())
	{
#ifdef _DEBUG // Debug Mode
	#ifdef _WIN64 // x64
		shaderfolder = L"..\\x64\\Debug\\";
	#else	// x86
		shaderfolder = L"..\\Debug\\";
	#endif
#else	// Release Mode
	#ifdef _WIN64
		shaderfolder = L"..\\x64\\Release\\";
	#else	// x86
		shaderfolder = L"..\\Release\\";
	#endif
#endif
	}


	
	d3dvertexshader = std::make_unique<D3DVertexShader>(device.Get(), StringHelper::WideToString(shaderfolder) + "vertexShader.cso");
	d3dvertexshader_animation = std::make_unique<D3DVertexShader>(device.Get(), StringHelper::WideToString(shaderfolder) + "VertexShaderAnim.cso");
	d3dvertexshader_nolight = std::make_unique<D3DVertexShader>(device.Get(), StringHelper::WideToString(shaderfolder) + "VS_nolight.cso");
	d3dvertexshader_shadowmap = std::make_unique<D3DVertexShader>(device.Get(), StringHelper::WideToString(shaderfolder) + "VS_shadowmap.cso");
	d3dvertexshader_shadowmap_anim = std::make_unique<D3DVertexShader>(device.Get(), StringHelper::WideToString(shaderfolder) + "VS_shadowmap_anim.cso");
	d3dvertexshader_dynamic_sky = std::make_unique<D3DVertexShader>(device.Get(), StringHelper::WideToString(shaderfolder) + "VS_DynamicSky.cso");
	if (!pixelshader.Initialize(this->device, shaderfolder + L"pixelshader.cso"))
	{
		return false;
	}
	if (!pixelshader_nolight.Initialize(this->device, shaderfolder + L"pixelshader_nolight.cso"))
	{
		return false;
	}
	if (!pixelshader_tonemapping.Initialize(this->device, shaderfolder + L"pixelshader_tonemapping.cso"))
	{
		return false;
	}
	if (!pixelshader_heightmapping.Initialize(this->device, shaderfolder + L"PixelShader_HeightMapping.cso"))
	{
		return false;
	}
	if (!pixelshader_dynamic_sky.Initialize(this->device, shaderfolder + L"PS_DynamicSky.cso"))
	{
		return false;
	}

	return true;
}

bool Graphics::InitializeScene()
{
	
	

	HRESULT hr;
		// load texture
	// hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\seamless_grass.jpg", nullptr, grassTexture.GetAddressOf());
	// COM_ERROR_IF_FAILED(hr, "Failed to create wic texture.");
	// 
	// hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\pinksquare.jpg", nullptr, pinkTexture.GetAddressOf());
	// COM_ERROR_IF_FAILED(hr, "Failed to create wic texture.");
	// 
	// hr = DirectX::CreateWICTextureFromFile(this->device.Get(), L"Data\\Textures\\seamless_pavement.jpg", nullptr, pavementTexture.GetAddressOf());
	// COM_ERROR_IF_FAILED(hr, "Failed to create wic texture.");
	hr = DirectX::CreateWICTextureFromFile(device.Get(), L"Data\\Textures\\tone_.png", nullptr, toneTexture.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create wic texture.");

	//initialize constant buffer
	hr = this->cb_vs_vertexshader.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");
	cb_vs_vertexshader.SetDebugName("cb_vs_world");

	hr = this->cb_ps_light.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");
	cb_ps_light.SetDebugName("cb_ps_light");

	hr = this->cb_ps_shadowmat.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");


	hr = this->cb_ps_common.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");
	cb_ps_common.SetDebugName("cb_ps_common");

	hr = this->cb_ps_material.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");
	cb_ps_material.SetDebugName("ps_material");

	hr = this->cb_bones.Initialize(this->device.Get(), this->deviceContext.Get());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");
	cb_bones.SetDebugName("vs_bone_transforms");

	// Initialize Model(s)
	
	cb_ps_light.data.ambientLightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
	cb_ps_light.data.ambientLightStrength = 0.04f;
	

	if (!gameObj.Initialize("Data\\Objects\\akai_e_espiritu@Taunt.fbx", this->device.Get(), this->deviceContext.Get(),
		cb_vs_vertexshader, cb_ps_material, d3dvertexshader_animation.get()))
	{
		COM_ERROR_IF_FAILED(-1, "Failed to load model file.");
		return false;
	}
	gameObj.SetScale(0.07f, 0.07f, 0.07f);
	gameObj.AdjustPosition(0.0f, 0.0f, 3.0f);
	gameObj.InitAnimation(cb_bones);

	if (!gameObj2.Initialize("Data\\Objects\\brick_wall\\brick_wall.obj", this->device.Get(), this->deviceContext.Get(), cb_vs_vertexshader, cb_ps_material, d3dvertexshader.get()))
	{
		COM_ERROR_IF_FAILED(-1, "Failed to load model file.");
		return false;
	}
	gameObj2.AdjustRotation(DirectX::XMConvertToRadians(90.0f), 0.0f, 0.0f);
	gameObj2.SetScale(10.0f, 10.0f, 10.0f);

	if (!gameObj3.Initialize("Data\\Objects\\sphere_smooth.obj", this->device.Get(), this->deviceContext.Get(), cb_vs_vertexshader, cb_ps_material, d3dvertexshader.get()))
	{
		COM_ERROR_IF_FAILED(-1, "Failed to load model file.");
		return false;
	}
	gameObj3.AdjustPosition(3.0f, 3.0f, 0.0f);

	if (!light.Initialize(this->device.Get(), this->deviceContext.Get(), cb_vs_vertexshader, cb_ps_material, d3dvertexshader_nolight.get()))
	{
		COM_ERROR_IF_FAILED(-1, "Failed to load model file.");
		return false;
	}
	light.AdjustPosition(0.0f, 10.0f, -6.0f);
	light.AdjustRotation(DirectX::XMConvertToRadians(20.0f), 0.0f, 0.0f);


	Camera3D.SetPosition(0.0f, 5.0f, -2.0f);
	Camera3D.SetProjectionValues(90.0f, static_cast<float>(window_width) / static_cast<float>(window_height), 0.1f, 1000.0f);
	
	return true;
}
