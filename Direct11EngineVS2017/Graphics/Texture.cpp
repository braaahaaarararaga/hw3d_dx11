#include <fstream>
#include "Texture.h"
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
		this->Initialize1x1ColorTexture(pDevice, Hw3d_Colors::UnloadedTextureColor, type);
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

D3DTexture::D3DTexture(ID3D11Device * pDevice, const Color & color, aiTextureType type)
{
	Initialize1x1ColorTexture(pDevice, color, type);
}

D3DTexture::D3DTexture(ID3D11Device * pDevice, const Color * colorData, UINT width, UINT height, aiTextureType type)
{
	InitializeColorTexture(pDevice, colorData, width, height, type);
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

Texture::Texture(ID3D11Device * device, const std::string & filename, aiTextureType type)
	:
	m_pTexture(new D3DTexture(device, filename, type))
{}

Texture::Texture(ID3D11Device* device, const char* pData, size_t size, aiTextureType type)
	:
	m_pTexture(new D3DTexture(device, pData, size, type))
{}

Texture::Texture(ID3D11Device * device, const Color * colorData, UINT width, UINT height, aiTextureType type)
	:
	m_pTexture(new D3DTexture(device, colorData, width, height, type))
{
}

Texture::Texture(ID3D11Device * device, const Color & color, aiTextureType type)
	:
	m_pTexture(new D3DTexture(device, color, type))
{
}

aiTextureType Texture::GetType()
{
	return m_pTexture->GetType();
}

ID3D11ShaderResourceView * const * Texture::GetShaderResourceView() const
{ 
	return m_pTexture->GetShaderResourceView(); 
}

Texture::operator ITexture*()
{
	return m_pTexture.get();
}

ITexture* Texture::operator->()
{
	return m_pTexture.get();
}

const ITexture* Texture::operator->() const
{
	return m_pTexture.get();
}