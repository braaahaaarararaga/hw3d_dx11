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

struct CB_PS_shadowmat
{
	DirectX::XMMATRIX shadowMatrix;
};

struct CB_PS_material
{
	DirectX::XMFLOAT4  GlobalAmbient;

	DirectX::XMFLOAT4  AmbientColor;

	DirectX::XMFLOAT4  EmissiveColor;

	DirectX::XMFLOAT4  DiffuseColor;

	DirectX::XMFLOAT4  SpecularColor;

	DirectX::XMFLOAT4  Reflectance;

	float   Opacity;
	float   SpecularPower;
	float   IndexOfRefraction;
	int     HasAmbientTexture;

	int     HasEmissiveTexture;
	int     HasDiffuseTexture;
	int     HasSpecularTexture;
	int     HasSpecularPowerTexture;

	int     HasNormalTexture;
	int     HasBumpTexture;
	int     HasOpacityTexture;
	float   BumpIntensity;

	float   SpecularScale;
	float   AlphaThreshold;
	float   _pad1[2];
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