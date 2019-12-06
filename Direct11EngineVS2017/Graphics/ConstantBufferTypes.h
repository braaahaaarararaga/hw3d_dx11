#pragma once
#include <DirectXMath.h>

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX vpMatrix;
	DirectX::XMMATRIX worldMatrix;
};

struct CB_VS_vertexshader_2d
{
	DirectX::XMMATRIX wvpMatrix;
};

struct CB_PS_light
{
	DirectX::XMFLOAT3 ambientLightColor;
	float ambientLightStrength;

	DirectX::XMFLOAT3 dynamicLightColor;
	float dynamicLightStrength;
	DirectX::XMFLOAT3 dynamicPosition;
	float dynamicLightAttenuation_a;
	float dynamicLightAttenuation_b;
	float dynamicLightAttenuation_c;
};

struct CB_PS_common
{
	DirectX::XMFLOAT3 eyePos;
	float pad;
};