#include "Texture.h"
#include <fstream>
#include "..\\ErrorLogger.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

D3DTexture::D3DTexture(ID3D11Device* pDevice, const std::string& filename, aiTextureType type)
{
	m_Type = type;
	std::wstring wfilename = StringHelper::StringToWide(filename);

	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	pDevice->GetImmediateContext(&pContext);


	if (!std::ifstream(filename))
	{
		COM_ERROR_IF_FAILED(
			DirectX::CreateWICTextureFromFile(pDevice, nullptr, L"Assets/error.png", &m_pTexture, &m_pTextureView), "Failed to create error texture."
		);
	}
	else if (StringHelper::GetFileExtension(filename) != "dds")
	{
		COM_ERROR_IF_FAILED(
			DirectX::CreateWICTextureFromFileEx(pDevice, pContext.Get(), wfilename.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, DirectX::WIC_LOADER_IGNORE_SRGB, &m_pTexture, &m_pTextureView), "Failed to create texture."
		);

	}
	else
	{
		DirectX::DDS_ALPHA_MODE alpha_mode;

		COM_ERROR_IF_FAILED(
			DirectX::CreateDDSTextureFromFile(pDevice, pContext.Get(), wfilename.c_str(), &m_pTexture, &m_pTextureView, 0, &alpha_mode),
			"Failed to create dds texture"
		);

		m_bIsTranslucent = (alpha_mode == DirectX::DDS_ALPHA_MODE_UNKNOWN || alpha_mode == DirectX::DDS_ALPHA_MODE_STRAIGHT);
	}

	// get width/height
	Microsoft::WRL::ComPtr<ID3D11Resource> pResource;
	m_pTextureView->GetResource(&pResource);
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture2D;
	pResource.As(&pTexture2D);
	D3D11_TEXTURE2D_DESC desc;
	pTexture2D->GetDesc(&desc);
	m_Format = (TexFormat)desc.Format;
	m_iWidth = desc.Width;
	m_iHeight = desc.Height;
}

D3DTexture::D3DTexture(ID3D11Device* pDevice, const char* pData, size_t size, aiTextureType type)
{
	m_Type = type;
	COM_ERROR_IF_FAILED(
		DirectX::CreateWICTextureFromMemory(pDevice,
			nullptr,
			reinterpret_cast<const uint8_t*>(pData),
			size,
			&m_pTexture,
			&m_pTextureView,
			0), "Failed create texture from memory"
	);

	// get width/height
	Microsoft::WRL::ComPtr<ID3D11Resource> pResource;
	m_pTextureView->GetResource(&pResource);
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture2D;
	pResource.As(&pTexture2D);

	D3D11_TEXTURE2D_DESC desc;
	pTexture2D->GetDesc(&desc);
	m_iWidth = desc.Width;
	m_iHeight = desc.Height;
	m_Format = (TexFormat)desc.Format;

}

D3DTexture::D3DTexture(ID3D11Device* pDevice, const void* pPixels, size_t pitch, size_t width, size_t height, TexFormat format, GPUResourceUsage usage)
{
	UINT flags = 0;
	if (usage == USAGE_DYNAMIC)
	{
		flags = D3D11_CPU_ACCESS_WRITE;
	}

	CD3D11_TEXTURE2D_DESC textureDesc((DXGI_FORMAT)format, (UINT)width, (UINT)height, 1, 1, D3D11_BIND_SHADER_RESOURCE, (D3D11_USAGE)usage, flags);
	ID3D11Texture2D* p2DTexture;

	if (pPixels)
	{
		D3D11_SUBRESOURCE_DATA initialData{};
		initialData.pSysMem = pPixels;
		initialData.SysMemPitch = (UINT)pitch;
		COM_ERROR_IF_FAILED(pDevice->CreateTexture2D(&textureDesc, &initialData, &p2DTexture), "Failed to create texture2d");
	}
	else
	{
		COM_ERROR_IF_FAILED(pDevice->CreateTexture2D(&textureDesc, nullptr, &p2DTexture), "Failed to create texture2d");
	}

	m_pTexture = static_cast<ID3D11Resource*>(p2DTexture);

	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
	COM_ERROR_IF_FAILED(
		pDevice->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, &m_pTextureView), "Failed to create shader res view"
	);

	m_iWidth = width;
	m_iHeight = height;
	m_Format = format;

	m_bIsTranslucent = (TexFormatInfo(m_Format).num_alpha_bits > 0);
}

void D3DTexture::Initialize1x1ColorTexture(ID3D11Device * device, const Color & colorData, aiTextureType type)
{
	InitializeColorTexture(device, &colorData, 1, 1, type);
}

void D3DTexture::InitializeColorTexture(ID3D11Device * device, const Color * colorData, UINT width, UINT height, aiTextureType type)
{
	m_Type = type;
	CD3D11_TEXTURE2D_DESC textureDesc(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
	ID3D11Texture2D* p2DTexture = nullptr;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = colorData;
	initialData.SysMemPitch = width * sizeof(Color);
	HRESULT hr = device->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
	COM_ERROR_IF_FAILED(hr, "Failed to initialize texture from color data.");
	m_pTexture = static_cast<ID3D11Texture2D*>(p2DTexture);
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
	hr = device->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, m_pTextureView.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create shader resource view from texture genarated from color data.");
}


static TexFormatInfo_t g_TexFormatInfo[] =
{
	{ "UNKNOWN", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_UNKNOWN
	{ "R32G32B32A32_TYPELESS", 16, 32, 32, 32, 32, false }, // TEX_FORMAT_R32G32B32A32_TYPELESS
	{ "R32G32B32A32_FLOAT", 16, 32, 32, 32, 32, false }, // TEX_FORMAT_R32G32B32A32_FLOAT
	{ "R32G32B32A32_UINT", 16, 32, 32, 32, 32, false }, // TEX_FORMAT_R32G32B32A32_UINT
	{ "R32G32B32A32_SINT", 16, 32, 32, 32, 32, false }, // TEX_FORMAT_R32G32B32A32_SINT
	{ "R32G32B32_TYPELESS",12, 32, 32, 32, 0, false }, // TEX_FORMAT_R32G32B32_TYPELESS
	{ "R32G32B32_FLOAT", 12, 32, 32, 32, 0, false }, // TEX_FORMAT_R32G32B32_FLOAT
	{ "R32G32B32_UINT", 12, 32, 32, 32, 0, false }, // TEX_FORMAT_R32G32B32_UINT
	{ "R32G32B32_SINT", 12, 32, 32, 32, 0, false }, // TEX_FORMAT_R32G32B32_SINT
	{ "R16G16B16A16_TYPELESS", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_TYPELESS
	{ "R16G16B16A16_FLOAT", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_FLOAT
	{ "R16G16B16A16_UNORM", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_UNORM
	{ "R16G16B16A16_UINT", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_UINT
	{ "R16G16B16A16_SNORM", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_SNORM
	{ "R16G16B16A16_SINT", 8, 16, 16, 16, 16, false }, // TEX_FORMAT_R16G16B16A16_SINT
	{ "R32G32_TYPELESS", 8, 32, 32, 0, 0, false }, // TEX_FORMAT_R32G32_TYPELESS
	{ "R32G32_FLOAT", 8, 32, 32, 0, 0, false }, // TEX_FORMAT_R32G32_FLOAT
	{ "R32G32_UINT", 8, 32, 32, 0, 0, false }, // TEX_FORMAT_R32G32_UINT
	{ "R32G32_SINT", 8, 32, 32, 0, 0, false }, // TEX_FORMAT_R32G32_SINT
	{ "R32G8X24_TYPELESS", 8, 32, 8, 0, 24, false }, // TEX_FORMAT_R32G8X24_TYPELESS
	{ "D32_FLOAT_S8X24_UINT", 8, 0, 0, 0, 0, false }, // TEX_FORMAT_D32_FLOAT_S8X24_UINT
	{ "R32_FLOAT_X8X24_TYPELESS", 8, 32, 0, 0, 0, false }, // TEX_FORMAT_R32_FLOAT_X8X24_TYPELESS
	{ "X32_TYPELESS_G8X24_UINT", 8, 0, 8, 0, 0, false }, // TEX_FORMAT_X32_TYPELESS_G8X24_UINT
	{ "R10G10B10A2_TYPELESS", 4, 10, 10, 10, 2, false }, // TEX_FORMAT_R10G10B10A2_TYPELESS
	{ "R10G10B10A2_UNORM", 4, 10, 10, 10, 2, false }, // TEX_FORMAT_R10G10B10A2_UNORM
	{ "R10G10B10A2_UINT", 4, 10, 10, 10, 2, false }, // TEX_FORMAT_R10G10B10A2_UINT
	{ "R11G11B10_FLOAT", 4, 11, 11, 10, 0, false }, // TEX_FORMAT_R11G11B10_FLOAT
	{ "R8G8B8A8_TYPELESS", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_TYPELESS
	{ "R8G8B8A8_UNORM", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_UNORM
	{ "R8G8B8A8_UNORM_SRGB", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_UNORM_SRGB
	{ "R8G8B8A8_UINT", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_UINT
	{ "R8G8B8A8_SNORM", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_SNORM
	{ "R8G8B8A8_SINT", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_R8G8B8A8_SINT
	{ "R16G16_TYPELESS", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_TYPELESS
	{ "R16G16_FLOAT", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_FLOAT
	{ "R16G16_UNORM", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_UNORM
	{ "R16G16_UINT", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_UINT
	{ "R16G16_SNORM", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_SNORM
	{ "R16G16_SINT", 4, 16, 16, 0, 0, false }, // TEX_FORMAT_R16G16_SINT
	{ "R32_TYPELESS", 4, 32, 0, 0, 0, false }, // TEX_FORMAT_R32_TYPELESS
	{ "D32_FLOAT", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_D32_FLOAT
	{ "R32_FLOAT", 4, 32, 0, 0, 0, false }, // TEX_FORMAT_R32_FLOAT
	{ "R32_UINT", 4, 32, 0, 0, 0, false }, // TEX_FORMAT_R32_UINT
	{ "R32_SINT", 4, 32, 0, 0, 0, false }, // TEX_FORMAT_R32_SINT
	{ "R24G8_TYPELESS", 4, 24, 8, 0, 0, false }, // TEX_FORMAT_R24G8_TYPELESS
	{ "D24_UNORM_S8_UINT", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_D24_UNORM_S8_UINT
	{ "R24_UNORM_X8_TYPELESS", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_R24_UNORM_X8_TYPELESS
	{ "X24_TYPELESS_G8_UINT", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_X24_TYPELESS_G8_UINT
	{ "R8G8_TYPELESS", 2, 8, 8, 0, 0, false }, // TEX_FORMAT_R8G8_TYPELESS
	{ "R8G8_UNORM", 2, 8, 8, 0, 0, false }, // TEX_FORMAT_R8G8_UNORM
	{ "R8G8_UINT", 2, 8, 8, 0, 0, false }, // TEX_FORMAT_R8G8_UINT
	{ "R8G8_SNORM", 2, 8, 8, 0, 0, false }, // TEX_FORMAT_R8G8_SNORM
	{ "R8G8_SINT", 2, 8, 8, 0, 0, false }, // TEX_FORMAT_R8G8_SINT
	{ "R16_TYPELESS", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_TYPELESS
	{ "R16_FLOAT", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_FLOAT
	{ "D16_UNORM", 2, 0, 0, 0, 0, false }, // TEX_FORMAT_D16_UNORM
	{ "R16_UNORM", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_UNORM
	{ "R16_UINT", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_UINT
	{ "R16_SNORM", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_SNORM
	{ "R16_SINT", 2, 16, 0, 0, 0, false }, // TEX_FORMAT_R16_SINT
	{ "R8_TYPELESS", 1, 8, 0, 0, 0, false }, // TEX_FORMAT_R8_TYPELESS
	{ "R8_UNORM", 1, 8, 0, 0, 0, false }, // TEX_FORMAT_R8_UNORM
	{ "R8_UINT", 1, 8, 0, 0, 0, false }, // TEX_FORMAT_R8_UINT
	{ "R8_SNORM", 1, 8, 0, 0, 0, false }, // TEX_FORMAT_R8_SNORM
	{ "R8_SINT", 1, 8, 0, 0, 0, false }, // TEX_FORMAT_R8_SINT
	{ "A8_UNORM", 1, 0, 0, 0, 8, false }, // TEX_FORMAT_A8_UNORM
	{ "R1_UNORM", 0, 1, 0, 0, 0, false }, // TEX_FORMAT_R1_UNORM
	{ "R9G9B9E5_SHAREDEXP", 4, 9, 9, 9, 0, false }, // TEX_FORMAT_R9G9B9E5_SHAREDEXP
	{ "R8G8_B8G8_UNORM", 4, 8, 16, 8, 0, false }, // TEX_FORMAT_R8G8_B8G8_UNORM
	{ "G8R8_G8B8_UNORM", 4, 8, 16, 8, 0, false }, // TEX_FORMAT_G8R8_G8B8_UNORM
	{ "BC1_TYPELESS", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC1_TYPELESS
	{ "BC1_UNORM", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC1_UNORM
	{ "BC1_UNORM_SRGB", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC1_UNORM_SRGB
	{ "BC2_TYPELESS", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC2_TYPELESS
	{ "BC2_UNORM", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC2_UNORM
	{ "BC2_UNORM_SRGB", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC2_UNORM_SRGB
	{ "BC3_TYPELESS", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC3_TYPELESS
	{ "BC3_UNORM", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC3_UNORM
	{ "BC3_UNORM_SRGB", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC3_UNORM_SRGB
	{ "BC4_TYPELESS", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC4_TYPELESS
	{ "BC4_UNORM", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC4_UNORM
	{ "BC4_SNORM", 8, 0, 0, 0, 0, true }, // TEX_FORMAT_BC4_SNORM
	{ "BC5_TYPELESS", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC5_TYPELESS
	{ "BC5_UNORM", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC5_UNORM
	{ "BC5_SNORM", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC5_SNORM
	{ "B5G6R5_UNORM", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_B5G6R5_UNORM
	{ "B5G5R5A1_UNORM", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_B5G5R5A1_UNORM
	{ "B8G8R8A8_UNORM", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_B8G8R8A8_UNORM
	{ "B8G8R8X8_UNORM", 4, 8, 8, 8, 0, false }, // TEX_FORMAT_B8G8R8X8_UNORM
	{ "R10G10B10_XR_BIAS_A2_UNORM", 4, 10, 10, 10, 2, false }, // TEX_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
	{ "B8G8R8A8_TYPELESS", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_B8G8R8A8_TYPELESS
	{ "B8G8R8A8_UNORM_SRGB", 4, 8, 8, 8, 8, false }, // TEX_FORMAT_B8G8R8A8_UNORM_SRGB
	{ "B8G8R8X8_TYPELESS", 4, 8, 8, 8, 0, false }, // TEX_FORMAT_B8G8R8X8_TYPELESS
	{ "B8G8R8X8_UNORM_SRGB", 4, 8, 8, 8, 0, false }, // TEX_FORMAT_B8G8R8X8_UNORM_SRGB
	{ "BC6H_TYPELESS", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC6H_TYPELESS
	{ "BC6H_UF16", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC6H_UF16
	{ "BC6H_SF16", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC6H_SF16
	{ "BC7_TYPELESS", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC7_TYPELESS
	{ "BC7_UNORM", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC7_UNORM
	{ "BC7_UNORM_SRGB", 16, 0, 0, 0, 0, true }, // TEX_FORMAT_BC7_UNORM_SRGB
	{ "AYUV", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_AYUV
	{ "Y410", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_Y410
	{ "Y416", 8, 0, 0, 0, 0, false }, // TEX_FORMAT_Y416
	{ "NV12", 2, 0, 0, 0, 0, false }, // TEX_FORMAT_NV12
	{ "P010", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_P010
	{ "P016", 4, 0, 0, 0, 0, false }, // TEX_FORMAT_P016
	{ "420_OPAQUE", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_420_OPAQUE
	{ "YUY2", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_YUY2
	{ "Y210", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_Y210
	{ "Y216", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_Y216
	{ "NV11", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_NV11
	{ "AI44", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_AI44
	{ "IA44", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_IA44
	{ "P8", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_P8
	{ "A8P8", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_A8P8
	{ "B4G4R4A4_UNORM", 2, 4, 4, 4, 4, false }, // TEX_FORMAT_B4G4R4A4_UNORM
	{ "P208", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_P208
	{ "V208", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_V208
	{ "V408", 0, 0, 0, 0, 0, false }, // TEX_FORMAT_V408
};

const TexFormatInfo_t& TexFormatInfo(TexFormat format)
{
	return g_TexFormatInfo[format];
}