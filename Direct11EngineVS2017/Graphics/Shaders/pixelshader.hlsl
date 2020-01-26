#pragma pack_matrix( row_major )
#include "CommonPS.hlsl"

cbuffer lightBuffer : register(b0)
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

cbuffer commonBuffer : register(b1)
{
    float3 eyePos;
    float pad;
}

cbuffer Material : register(b2)
{
    Material Mat;
}

cbuffer Matrices : register(b3)
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

Texture2D diffuseTexture : TEXTURE: register(t0);
Texture2D normalTexture : TEXTURE: register(t1);
Texture2D specularTexture : TEXTURE : register(t2);
Texture2D emissiveTexture : TEXTURE : register(t3);
Texture2D shadowTexture : TEXTURE : register(t4);

SamplerState objSamplerState : SAMPLER: register(s0);
SamplerState shadowSamplerState : SAMPLER: register(s1);


float Shadow(float3 worldPos, float3 normal, float3 light_dir)
{
    float4 proj_coords = mul(float4(worldPos, 1.0f), ShadowMatrix);
    proj_coords.xyz /= proj_coords.w;
	
    float2 shadow_uv = proj_coords.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
    float closest_depth = shadowTexture.Sample(shadowSamplerState, shadow_uv).r;
    float current_depth = proj_coords.z;
	
   
    float bias = 0.00005f;
    float shadow = current_depth - bias > closest_depth ? 0.0f : 1.0f;
	
    return shadow;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    Material material = Mat;
    float4 inpos = input.inPosition;
    float3 normal = input.inNormal;
    // do normal mapping
    if(material.HasNormalTexture)
    {
        float3 normal_sample = normalTexture.Sample(objSamplerState, input.inTexCoord).xyz;
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
        material.DiffuseColor = diffuseTexture.Sample(objSamplerState, input.inTexCoord);
    }
    //sampleColor = float3(1,1,1);
    
    if (material.HasSpecularTexture)
    {
        material.SpecularColor = specularTexture.Sample(objSamplerState, input.inTexCoord);
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
        float3 sample = emissiveTexture.Sample(objSamplerState, input.inTexCoord).rgb;
		
        if (any(emissive))
        {
            emissive *= emissiveTexture.Sample(objSamplerState, input.inTexCoord).rgb;
        }
        else
        {
            emissive = emissiveTexture.Sample(objSamplerState, input.inTexCoord).rgb;
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
    float3 eyev = normalize(eyePos - worldPos);
    float rv = dot(-refv, eyev);
    rv = saturate(rv);
    float3 specular = material.SpecularColor.rgb * pow(rv, material.SpecularPower);
    
    diffuseLightIntensity = (diffuseLightIntensity + specular) * attenuationFactor * Shadow(worldPos, normal, vectorToLight);
    float3 diffuseLight = diffuseLightIntensity * dynamicStrength * dynamicLight;
    appliedLight += diffuseLight;
    
    
    
    finalColor = material.DiffuseColor.xyz * appliedLight;
    
    return float4(finalColor, 1.0f) * material.Opacity;
}

