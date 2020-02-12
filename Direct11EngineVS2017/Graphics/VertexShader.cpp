#include "VertexShader.h"
#include <assert.h>
#include "../MemoryStream.h"

D3DVertexShader::D3DVertexShader(ID3D11Device* pDevice, const std::string& filename, const std::vector<ShaderMacro>& macros)
	:
	m_szName(filename),
	m_Macros(macros)
{
	LoadFromFile(pDevice, filename, true);
}

D3DVertexShader::~D3DVertexShader()
{
}

ID3D11VertexShader* D3DVertexShader::GetShader(ID3D11Device* pDevice)
{
	if (IsDirty())
	{
		LoadFromFile(pDevice, m_szName, false);

		SetDirty(false);
	}

	return m_pShader.Get();
}

void D3DVertexShader::CreateInputLayoutDescFromVertexShaderSignature(ID3D11Device* pDevice, const void* pCodeBytes, const size_t size)
{
	// Reflect shader info 
	ComPtr<ID3D11ShaderReflection> pVertexShaderReflection;

	COM_ERROR_IF_FAILED(
		D3DReflect(pCodeBytes, size, IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection),
		"Failed to get d3dreflect"
	);
	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	pVertexShaderReflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (UINT i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = i;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		auto attribute = AttributeInfo::SemanticToAttribute(paramDesc.SemanticName);
		if (attribute == AttributeInfo::VertexAttribute::Invalid)
		{
			COM_ERROR_IF_FAILED(-1, "Unknown semantic type");
			return;
		}
		m_bUsesAttribute[(int)attribute] = true;

		// determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	pDevice->CreateInputLayout(inputLayoutDesc.data(), (UINT)inputLayoutDesc.size(), pCodeBytes, size, &m_pInputLayout);
}

void D3DVertexShader::LoadFromFile(ID3D11Device* pDevice, const std::string& filename, bool crash_on_error)
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
	for (int i = 0; i < (int)AttributeInfo::VertexAttribute::TotalAttributes; i++)
	{
		m_bUsesAttribute[i] = false;
	}

	// compiled shader object
	if (StringHelper::GetFileExtension(StringHelper::WideToString(shaderfolder)) == "cso")
	{
		auto bytes = MemoryStream::FromFile(shaderfolder);

		COM_ERROR_IF_FAILED(pDevice->CreateVertexShader(bytes.Base(), bytes.Size(), NULL, &m_pShader), "Failed to Create Shader");
		CreateInputLayoutDescFromVertexShaderSignature(pDevice, bytes.Base(), bytes.Size());
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

		ComPtr<ID3DBlob> errorMessage;
		ComPtr<ID3DBlob> vertexShaderBuffer;

		UINT flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif



		COM_ERROR_IF_FAILED(D3DCompileFromFile(shaderfolder.c_str(), macros.data(), NULL, "main", "vs_5_0", flags, 0, &vertexShaderBuffer, &errorMessage),
			"Failed to compile shader");
		

		COM_ERROR_IF_FAILED(pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_pShader), "Failed to create shader");
		CreateInputLayoutDescFromVertexShaderSignature(pDevice, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize());
	}
}

void D3DVertexShader::OnFileChanged(const std::string & filename)
{
	SetDirty(true);
}