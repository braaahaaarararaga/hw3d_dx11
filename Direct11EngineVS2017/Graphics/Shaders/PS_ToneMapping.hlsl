#include "CommonPS.hlsli"

Texture2D SceneTexture : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 finalColor = SceneTexture.Sample(objSamplerState, input.tex);
    if(finalColor.r > 1.0 && finalColor.g > 1.0 && finalColor.b > 1.0)
        finalColor.gb = 0.0;
    return finalColor;
}