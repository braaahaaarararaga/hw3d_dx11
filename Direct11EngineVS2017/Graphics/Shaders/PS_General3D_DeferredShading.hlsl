#include "CommonPS.hlsli"

cbuffer Material : register(B_SLOT_MATERIAL)
{
    Material Mat;
}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float3 inWorldPos : WORLD_POSITION;
    float3 inNormal : NORMAL;
    float3 inTangent : TANGENT;
    float3 inBitangent : BITANGENT;
    float2 inTexCoord : TEXCOORD;
};

struct PS_OUTPUT
{
    float4 outPosition : SV_Target0;
    float4 outNormal : SV_Target1;
    float4 outAlbedo : SV_Target2;
    float4 outSpecColor : SV_Target3;
};


Texture2D diffuseTexture : TEXTURE : register(t0);
Texture2D normalTexture : TEXTURE : register(t1);
Texture2D specularTexture : TEXTURE : register(t2);
Texture2D emissiveTexture : TEXTURE : register(t3);

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;
    
    Material material = Mat;
    //float4 inpos = input.inPosition;
    float3 normal = input.inNormal;
    // do normal mapping
    if (material.HasNormalTexture)
    {
        float3 normal_sample = normalTexture.Sample(WrapSampler, input.inTexCoord).xyz;
        normal = normal_sample * 2.0 - 1.0;
        //normal.y = -normal.y;
        float3x3 tbn = float3x3(normalize(input.inTangent), normalize(input.inBitangent), normalize(input.inNormal));
        normal = mul(normal, tbn);
    }
    normal = normalize(normal);
    float3 worldPos = input.inWorldPos.xyz;
    
    // diffuse
    if (material.HasDiffuseTexture)
    {
        material.DiffuseColor = diffuseTexture.Sample(WrapSampler, input.inTexCoord);
        material.DiffuseColor = ToLinearSpace(material.DiffuseColor);
    }
    
    if (material.HasSpecularTexture)
    {
        material.SpecularColor = specularTexture.Sample(WrapSampler, input.inTexCoord);
    }
    
    output.outAlbedo = float4(material.DiffuseColor.xyz, 1.0);
    output.outNormal = float4(normal.xyz, 1.0);
    output.outPosition = float4(input.inWorldPos, 1.0);
    output.outSpecColor = float4(material.SpecularColor.xyz, material.SpecularPower / 255.0);
    
    return output;
}