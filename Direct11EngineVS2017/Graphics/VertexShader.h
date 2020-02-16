#pragma once
#include "../ErrorLogger.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include "ShaderUtils.h"
#include <atomic>

class IVertexShader
{
public:
	virtual ~IVertexShader() = default;

	virtual std::string GetName() const = 0;
	virtual bool RequiresVertexAttribute(AttributeInfo::VertexAttribute attribute) const = 0;
};



class D3DVertexShader : public IVertexShader
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	D3DVertexShader(ID3D11Device* pDevice, const std::string& filename, const std::vector<ShaderMacro>& macros);
	~D3DVertexShader();

	virtual std::string GetName() const override { return m_szName; }
	virtual bool RequiresVertexAttribute(AttributeInfo::VertexAttribute attribute) const override { return m_bUsesAttribute[(int)attribute]; }
	ID3D11VertexShader* GetShader(ID3D11Device* pDevice);
	ID3D11InputLayout* GetLayout() { return m_pInputLayout.Get(); }
	const ID3D11InputLayout* GetLayout() const { return m_pInputLayout.Get(); }
private:
	void CreateInputLayoutDescFromVertexShaderSignature(ID3D11Device* pDevice, const void* pCodeBytes, const size_t size);
	void LoadFromFile(ID3D11Device* pDevice, const std::string& filename, bool crash_on_error);
	void OnFileChanged(const std::string& filename);
	bool IsDirty() const { return m_bDirty; }
	void SetDirty(bool dirty) { m_bDirty = dirty; }
private:
	std::string m_szName;
	std::vector<ShaderMacro> m_Macros;
	ComPtr<ID3D11InputLayout>	m_pInputLayout;
	bool m_bUsesAttribute[(int)AttributeInfo::VertexAttribute::TotalAttributes];
	std::atomic_bool m_bDirty = true;
	ComPtr<ID3D11VertexShader> m_pShader;

};
