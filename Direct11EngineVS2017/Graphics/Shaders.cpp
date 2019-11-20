#include "Shaders.h"

bool VertexShader_::Initialize(Microsoft::WRL::ComPtr<ID3D11Device>& device, std::wstring shaderpath,
	D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements)
{
	HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), this->shader_buffer.GetAddressOf());

	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	hr = device->CreateVertexShader(this->shader_buffer->GetBufferPointer(), this->shader_buffer->GetBufferSize(),
		NULL, this->shader.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to create vertex shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	hr = device->CreateInputLayout(layoutDesc, numElements, this->shader_buffer->GetBufferPointer(),
		this->shader_buffer->GetBufferSize(), this->inputLayout.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "FAILED TO CREATE INPUT LAYOUT");
		return false;
	}

	return true;
}

ID3D11VertexShader * VertexShader_::GetShader()
{
	return this->shader.Get();
}

ID3D10Blob * VertexShader_::GetBuffer()
{
	return this->shader_buffer.Get();
}

ID3D11InputLayout * VertexShader_::GetInputLayout()
{
	return this->inputLayout.Get();
}

bool PixelShader::Initialize(Microsoft::WRL::ComPtr<ID3D11Device>& device, std::wstring shaderpath)
{
	HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), this->shader_buffer.GetAddressOf());

	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	hr = device->CreatePixelShader(this->shader_buffer->GetBufferPointer(), this->shader_buffer->GetBufferSize(),
		NULL, this->shader.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to create vertex shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	return true;
}

ID3D11PixelShader * PixelShader::GetShader()
{
	return this->shader.Get();
}

ID3D10Blob * PixelShader::GetBuffer()
{
	return this->shader_buffer.Get();
}
