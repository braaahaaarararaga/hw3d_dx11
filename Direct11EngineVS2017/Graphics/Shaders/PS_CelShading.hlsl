#include "CommonPS.hlsli"

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


cbuffer Material : register(B_SLOT_MATERIAL)
{
    Material Mat;
}

cbuffer Matrices : register(B_SLOT_SHADOW)
{
    float4x4 ShadowMatrix;
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

Texture2D diffuseTexture : TEXTURE : register(t0);
Texture2D normalTexture : TEXTURE : register(t1);
Texture2D specularTexture : TEXTURE : register(t2);
Texture2D emissiveTexture : TEXTURE : register(t3);
Texture2D shadowTexture : TEXTURE : register(t4);
Texture2D toneTexture : TEXTURE : register(t5);



float Shadow(float3 worldPos, float3 normal, float3 light_dir)
{
    float4 proj_coords = mul(float4(worldPos, 1.0f), ShadowMatrix);
    proj_coords.xyz /= proj_coords.w;
	
    float2 shadow_uv = proj_coords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    //float closest_depth = shadowTexture.Sample(shadowSamplerState, shadow_uv).r;
    //float bias = max(0.005 * (1.0 - dot(normal, light_dir)), 0.0005f);
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


float4 main(PS_INPUT input) : SV_TARGET
{
    Material material = Mat;
    float4 inpos = input.inPosition;
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
    	// ambient
    float3 ambient = material.AmbientColor.rgb;
    if (material.HasAmbientTexture)
    {
        if (any(ambient))
        {
            ambient *= material.AmbientColor.rgb;
        }
        else
        {
            ambient = material.AmbientColor.rgb;
        }
    }

    ambient *= material.GlobalAmbient.rgb;
    ambient *= material.DiffuseColor.rgb;

	// emissive
    float3 emissive = material.EmissiveColor.rgb;
    if (material.HasEmissiveTexture)
    {
        float3 sample = emissiveTexture.Sample(WrapSampler, input.inTexCoord).rgb;
        sample = ToLinearSpace(sample);
        if (any(emissive))
        {
            emissive *= emissiveTexture.Sample(WrapSampler, input.inTexCoord).rgb;
        }
        else
        {
            emissive = emissiveTexture.Sample(WrapSampler, input.inTexCoord).rgb;
        }
    }
    
    float3 finalColor;
    
    // light
    float3 ambientLight = ambientLightColor * ambientLightStrength;
    float3 appliedLight = ambientLight;
    float3 vectorToLight = normalize(dynamicLightPosition - input.inWorldPos);
    float3 diffuseLightIntensity = max(dot(vectorToLight, normal), 0);
    float distanceToLight = distance(dynamicLightPosition, input.inWorldPos);
    float attenuationFactor = 1 / (dynamicLightAttenuation_a + dynamicLightAttenuation_b * distanceToLight + dynamicLightAttenuation_c * pow(distanceToLight, 2));
    
    float3 refv = reflect(vectorToLight, normal);
    refv = normalize(refv);
    float3 eyev = normalize(Globals.CameraPos - worldPos);
    float rv = dot(-refv, eyev);
    rv = saturate(rv);
    if(rv < 0.987)
        rv = 0.0;
    //else
    //    rv = 1.0;
    float3 specular = material.SpecularColor.rgb * pow(rv, material.SpecularPower);
    
    float light = dot(vectorToLight, normal) / 2.0 + 0.5;
    light *= Shadow(worldPos, normal, vectorToLight);
    float facing = dot(eyev, normal);
    float2 toneTexcoord = float2(light, facing);
    float3 toneColor = toneTexture.Sample(ClampSampler, toneTexcoord).rgb + specular;
    
    //diffuseLightIntensity = (diffuseLightIntensity + specular) * attenuationFactor * Shadow(worldPos, normal, vectorToLight);
    diffuseLightIntensity = toneColor * attenuationFactor;// * Shadow(worldPos, normal, vectorToLight);
    float3 diffuseLight = diffuseLightIntensity * dynamicStrength * dynamicLight;
    appliedLight += diffuseLight;
    
    
    
    finalColor = material.DiffuseColor.xyz * appliedLight;
    
    return float4(finalColor, 1.0f) * material.Opacity;
}

