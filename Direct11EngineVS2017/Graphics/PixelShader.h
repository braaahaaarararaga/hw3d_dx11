#pragma once
#include "../ErrorLogger.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <atomic>
#include "ShaderUtils.h"

class IPixelShader
{
public:
	virtual ~IPixelShader() = default;

	virtual std::string GetName() const = 0;
};

class D3DPixelShader : public IPixelShader
{
public:
	D3DPixelShader(ID3D11Device* pDevice, const std::string& filename, const std::vector<ShaderMacro>& macros);
	~D3DPixelShader();

	virtual std::string GetName() const override { return m_szName; }
	ID3D11PixelShader* GetShader(ID3D11Device* pDevice);
private:
	void LoadFromFile(ID3D11Device* pDevice, const std::string& filename, bool crash_on_error);
	void OnFileChanged(const std::string& filename);
	bool IsDirty() const { return m_bDirty; }
	void SetDirty(bool dirty) { m_bDirty = dirty; }
private:
	std::string m_szName;
	std::vector<ShaderMacro> m_Macros;
	std::atomic_bool m_bDirty = true;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pShader;
};