#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Color.h"
#include "..\\StringHelper.h"
#include <assimp/material.h>
#include "TextureFormat.h"

class ITexture
{
public:
	virtual ~ITexture() = default;

	virtual size_t GetWidth() const = 0;
	virtual size_t GetHeight() const = 0;
	virtual TexFormat GetFormat() const = 0;
	virtual aiTextureType GetType() const = 0;
	virtual bool IsTranslucent() const = 0;
	virtual ID3D11ShaderResourceView* const* GetShaderResourceView() const = 0;
};

class D3DTexture : public ITexture
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	D3DTexture(ID3D11Device* pDevice, const std::string& filename, aiTextureType type);
	D3DTexture(ID3D11Device* pDevice, const char* pData, size_t size, aiTextureType type);
	D3DTexture(ID3D11Device* pDevice, const Color& color, aiTextureType type);
	D3DTexture(ID3D11Device* pDevice, const Color* colorData, UINT width, UINT height, aiTextureType type);

	virtual size_t GetWidth() const override { return m_iWidth; }
	virtual size_t GetHeight() const override { return m_iHeight; }
	virtual TexFormat GetFormat() const override { return m_Format; }
	virtual aiTextureType GetType() const override { return m_Type; }
	virtual bool IsTranslucent() const override { return m_bIsTranslucent; }
	virtual ID3D11ShaderResourceView* const* GetShaderResourceView() const override { return m_pTextureView.GetAddressOf(); }
private:
	void Initialize1x1ColorTexture(ID3D11Device* device, const Color& colorData, aiTextureType type);
	void InitializeColorTexture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type);
private:
	ComPtr<ID3D11Resource>				m_pTexture = nullptr;
	ComPtr<ID3D11ShaderResourceView>	m_pTextureView = nullptr;

	TexFormat m_Format;
	aiTextureType m_Type = aiTextureType::aiTextureType_UNKNOWN;
	size_t m_iWidth = 0;
	size_t m_iHeight = 0;
	bool m_bIsTranslucent = false;
};

class Texture
{
public:
	Texture() = default;
	Texture(ID3D11Device * device, const std::string & filename, aiTextureType type);
	Texture(ID3D11Device * device, const std::wstring& filename, aiTextureType type)
		:
		Texture(device,StringHelper::WideToString(filename),type)
	{}
	Texture(ID3D11Device* device, const char* pData, size_t size, aiTextureType type);
	Texture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type);
	Texture(ID3D11Device* device, const Color& color, aiTextureType type);

	aiTextureType GetType();
	ID3D11ShaderResourceView* const* GetShaderResourceView() const;

	ITexture* Get() { return m_pTexture.get(); }
	const ITexture* Get() const { return m_pTexture.get(); }

	operator ITexture*();
	ITexture* operator->();
	const ITexture* operator->() const;
private:
	std::unique_ptr<ITexture> m_pTexture = nullptr;
};
