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

float4 main(PS_INPUT input) : SV_TARGET
{
    Material material = Mat;
    
    if (material.HasDiffuseTexture)
    {
        material.DiffuseColor = diffuseTexture.Sample(WrapSampler, input.inTexCoord);
    }
    float3 sampleColor = material.DiffuseColor * 1.5;

	return float4(sampleColor, 1.0f) * material.Opacity;
}