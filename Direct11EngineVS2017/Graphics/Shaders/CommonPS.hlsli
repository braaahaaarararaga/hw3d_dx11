#define PI 3.1415926254f

#define B_SLOT_LIGHT    b0
#define B_SLOT_GLOBALS  b1
#define B_SLOT_MATERIAL b2
#define B_SLOT_SHADOW   b3 

SamplerComparisonState sampler_cmp_depth : register(s0);
SamplerState WrapSampler : SAMPLER : register(s1);
SamplerState ClampSampler : SAMPLER : register(s2);

struct Material
{
    float4 GlobalAmbient;

    float4 AmbientColor;

    float4 EmissiveColor;

    float4 DiffuseColor;

    float4 SpecularColor;

    float4 Reflectance;

    float Opacity;
    float SpecularPower;
    float IndexOfRefraction;
    bool HasAmbientTexture;

    bool HasEmissiveTexture;
    bool HasDiffuseTexture;
    bool HasSpecularTexture;
    bool HasSpecularPowerTexture;

    bool HasNormalTexture;
    bool HasBumpTexture;
    bool HasOpacityTexture;
    float BumpIntensity;

    float SpecularScale;
    float AlphaThreshold;
    float2 Padding;
};

struct GlobalsBuf
{
	// width/height of screen
    float2 Resolution;
	// time in seconds since program was launched
    float Time;
	// time since last frame
    float DeltaTime;
	// camera position in world space
    float3 CameraPos;
    float pad0;
	
    float4x4 CameraInvVP;
};


// Global buffers
cbuffer GlobalsBuf : register(B_SLOT_GLOBALS)
{
    GlobalsBuf Globals;
};



#define GAMMA 2.2f

float3 TosRGBSpace(float3 color)
{
    return pow(color, 1.0 / GAMMA);
}

float4 TosRGBSpace(float4 color)
{
    return float4(TosRGBSpace(color.rgb), color.a);
}

float3 ToLinearSpace(float3 color)
{
    return pow(color, GAMMA);
}

float4 ToLinearSpace(float4 color)
{
    return float4(ToLinearSpace(color.rgb), color.a);
}