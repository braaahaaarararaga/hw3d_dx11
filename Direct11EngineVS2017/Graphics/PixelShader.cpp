#include "PixelShader.h"
#include "../MemoryStream.h"

D3DPixelShader::D3DPixelShader(ID3D11Device* pDevice, const std::string& filename, const std::vector<ShaderMacro>& macros)
	:
	m_szName(filename),
	m_Macros(macros)
{
	LoadFromFile(pDevice, filename, true);
}

D3DPixelShader::~D3DPixelShader()
{
}

ID3D11PixelShader* D3DPixelShader::GetShader(ID3D11Device* pDevice)
{
	if (IsDirty())
	{
		LoadFromFile(pDevice, m_szName, false);

		SetDirty(false);
	}

	return m_pShader.Get();
}

void D3DPixelShader::LoadFromFile(ID3D11Device* pDevice, const std::string& filename, bool crash_on_error)
{
	// compiled shader object
	if (StringHelper::GetFileExtension(filename) == "cso")
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
		shaderfolder = shaderfolder + StringHelper::StringToWide(filename);
		auto bytes = MemoryStream::FromFile(shaderfolder);
		COM_ERROR_IF_FAILED(pDevice->CreatePixelShader(bytes.Base(), bytes.Size(), NULL, &m_pShader),"Failed to create pixel shader from cso");
	}
	// not compiled, lets compile ourselves
	else
	{
		std::vector<D3D_SHADER_MACRO> macros;
		for (const ShaderMacro& macro : m_Macros)
		{
			macros.push_back({ macro.name.c_str(), macro.value.c_str() });
		}
		macros.push_back({ NULL, NULL }); // Sentinel

		HRESULT hr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorMessage;
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBuffer;

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		COM_ERROR_IF_FAILED(hr = D3DCompileFromFile(StringHelper::StringToWide(filename).c_str(), macros.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", flags, 0, &pixelShaderBuffer, &errorMessage), "Failed to compile shader");
		

		COM_ERROR_IF_FAILED(pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pShader), "Failed to create shader");
	}
}

void D3DPixelShader::OnFileChanged(const std::string& filename)
{
	SetDirty(true);
}