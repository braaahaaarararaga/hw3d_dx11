#include "CommonPS.hlsli"

cbuffer Material : register(B_SLOT_MATERIAL)
{
    Material Mat;
}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
};

Texture2D diffuseTexture : TEXTURE : register(t0);
Texture2D normalTexture : TEXTURE : register(t1);
Texture2D specularTexture : TEXTURE : register(t2);
Texture2D emissiveTexture : TEXTURE : register(t3);
SamplerState objSamplerState : SAMPLER: register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    Material material = Mat;
    
    if (material.HasDiffuseTexture)
    {
        material.DiffuseColor = diffuseTexture.Sample(objSamplerState, input.inTexCoord);
    }
    float3 sampleColor = material.DiffuseColor;

	return float4(sampleColor, 1.0f) * material.Opacity;
}