#pragma pack_matrix( row_major )

#define MAX_BONES 256

cbuffer SkeletalAnimationData : register(b1)
{
    float4x4 BoneTransforms[MAX_BONES];
};

cbuffer perObjectBuffer : register(b0)
{
    float4x4 vpMatrix;
    float4x4 worldMatrix;
};


struct VS_INPUT
{
    float3 inPos : POSITION;
    uint4 boneids : BONEID;
    float4 boneweights : BONEWEIGHT;
    float3 inNormal : NORMAL;
    float2 inTexCoord : TEXCOORD;
    float3 inTangent : TANGENT;
    float3 inBitangent : BITANGENT;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
    float4 outWorldPos : WORLD_POSITION;
    float3 outNormal : NORMAL;
    float3 outTangent : TANGENT;
    float3 outBitangent : BITANGENT;
    float2 outTexCoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 pos = float4(input.inPos, 1.0);
    float4 normal = float4(input.inNormal, 0.0);
    float4 tangent = float4(input.inTangent, 0.0);
    float4 bitangent = float4(input.inBitangent, 0.0);
    float4x4 bone_transform = mul(BoneTransforms[input.boneids.x], input.boneweights.x);
    bone_transform += mul(BoneTransforms[input.boneids.y], input.boneweights.y);
    bone_transform += mul(BoneTransforms[input.boneids.z], input.boneweights.z);
    bone_transform += mul(BoneTransforms[input.boneids.w], input.boneweights.w);
    
    tangent = mul(tangent, bone_transform);
    bitangent = mul(bitangent, bone_transform);
    
    pos = mul(pos, bone_transform);
    normal = mul(normal, bone_transform);
    
    output.outWorldPos = mul(pos, worldMatrix);
    output.outPosition = mul(output.outWorldPos, vpMatrix);
    output.outNormal = normalize(mul(normal, worldMatrix)).xyz;
    output.outTangent = normalize(mul(tangent, worldMatrix)).xyz;
    output.outBitangent = normalize(mul(bitangent, worldMatrix)).xyz;
    output.outTexCoord = input.inTexCoord;
    return output;
}