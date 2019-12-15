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