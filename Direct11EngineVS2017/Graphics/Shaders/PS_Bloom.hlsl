#include "CommonPS.hlsli"

Texture2D SceneTexture : register(t0);
Texture2D BlurTexture : register(t1);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PixelInputType input) : SV_TARGET
{
    float3 texColour = SceneTexture.Sample(ClampSampler, input.tex).rgb;
    float3 bloomColour = BlurTexture.Sample(ClampSampler, input.tex).rgb;
    return float4(texColour + bloomColour, 1.0f);
}