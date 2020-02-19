#include "CommonPS.hlsli"

 
Texture2D PositionTexture : TEXTURE : register(t0);
Texture2D NormalTexture : TEXTURE : register(t1);
Texture2D AlbedoTexture : TEXTURE : register(t2);
Texture2D SpecularTexture : TEXTURE : register(t3);
Texture2D shadowTexture : TEXTURE : register(t4);
Texture2D toneTexture : TEXTURE : register(t5);

cbuffer lightBuffer : register(B_SLOT_LIGHT)
{
    float3 ambientLightColor;
    float ambientLightStrength;

    float3 dynamicLight;
    float dynamicStrength;
    float3 dynamicLightPosition;

    float dynamicLightAttenuation_a;
    float dynamicLightAttenuation_b;
    float dynamicLightAttenuation_c;
}

cbuffer Matrices : register(B_SLOT_SHADOW)
{
    float4x4 ShadowMatrix;
}

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float Shadow(float3 worldPos, float3 normal, float3 light_dir)
{
    float4 proj_coords = mul(float4(worldPos, 1.0f), ShadowMatrix);
    proj_coords.xyz /= proj_coords.w;
	
    float2 shadow_uv = proj_coords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    float bias = 0.0004f;
    float current_depth = proj_coords.z - bias;
	
    float2 shadow = 0.0f;
    const float range = 1.5;
    float2 texelSize = 1.0 / 1024.0;

    for (float y = -range; y <= range; y += 1.0)
    {
        for (float x = -range; x <= range; x += 1.0)
        {
            shadow.x += shadowTexture.SampleCmpLevelZero(sampler_cmp_depth, shadow_uv + float2(x, y) * texelSize, current_depth).r;
            shadow.y++;
        }
    }
    shadow = shadow.x / shadow.y;
	
    return shadow.x;
}

float4 main(PixelInputType input) : SV_TARGET
{
    float3 finalColor;
    float3 worldPos = PositionTexture.Sample(ClampSampler, input.tex).xyz;
    float3 normal = NormalTexture.Sample(ClampSampler, input.tex).xyz;
    if (length(normal.xyz) < 0.1f)
        discard;
    float3 albedo = AlbedoTexture.Sample(ClampSampler, input.tex).rgb;
    float4 specularColor = SpecularTexture.Sample(ClampSampler, input.tex).rgba;
    float specPower = specularColor.a * 255;
    // light
    float3 ambientLight = ambientLightColor * ambientLightStrength;
    float3 appliedLight = ambientLight;
    float3 vectorToLight = normalize(dynamicLightPosition - worldPos);
    float3 diffuseLightIntensity = max(dot(vectorToLight, normal), 0);
    float distanceToLight = distance(dynamicLightPosition, worldPos);
    float attenuationFactor = 1 / (dynamicLightAttenuation_a + dynamicLightAttenuation_b * distanceToLight + dynamicLightAttenuation_c * pow(distanceToLight, 2));
    
    float3 refv = reflect(vectorToLight, normal);
    refv = normalize(refv);
    float3 eyev = normalize(Globals.CameraPos - worldPos);
    float rv = dot(-refv, eyev);
    rv = saturate(rv);
    if (rv < 0.987)
        rv = 0.0;
    //else
    //    rv = 1.0;
    float3 specular = specularColor.rgb * pow(rv, specPower);
    
    float light = dot(vectorToLight, normal) / 2.0 + 0.5;
    light *= Shadow(worldPos, normal, vectorToLight);
    float facing = dot(eyev, normal);
    float2 toneTexcoord = float2(light, facing);
    float3 toneColor = toneTexture.Sample(ClampSampler, toneTexcoord).rgb + specular;
    
    //diffuseLightIntensity = (diffuseLightIntensity + specular) * attenuationFactor * Shadow(worldPos, normal, vectorToLight);
    diffuseLightIntensity = toneColor * attenuationFactor; // * Shadow(worldPos, normal, vectorToLight);
    float3 diffuseLight = diffuseLightIntensity * dynamicStrength * dynamicLight;
    appliedLight += diffuseLight;
    
    
    
    finalColor = albedo.xyz * appliedLight;
    
    return float4(finalColor, 1.0f);
}