
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
#ifdef HAS_BONES
    uint4 boneids : BONEID;
    float4 boneweights : BONEWEIGHT;
#endif
    float3 inNormal : NORMAL;
    float2 inTexCoord : TEXCOORD;
#ifdef HAS_TANGENT
  float3 inTangent : TANGENT;
  float3 inBitangent : BITANGENT;
#endif
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
    float4 outWorldPos : WORLD_POSITION;
    float3 outNormal : NORMAL;
#ifdef HAS_TANGENT
  float3 outTangent : TANGENT;
  float3 outBitangent : BITANGENT;
#endif
    float2 outTexCoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 pos = float4(input.inPos, 1.0);
    float4 normal = float4(input.inNormal, 0.0);
#ifdef HAS_TANGENT
    float4 tangent = float4(input.inTangent, 0.0f);
    float4 bitangent = float4(input.inBitangent, 0.0f);
#endif
    float4x4 final_world = worldMatrix;
#ifdef HAS_BONES
    float4x4 bone_transform = mul(BoneTransforms[input.boneids.x], input.boneweights.x);
    bone_transform += mul(BoneTransforms[input.boneids.y], input.boneweights.y);
    bone_transform += mul(BoneTransforms[input.boneids.z], input.boneweights.z);
    bone_transform += mul(BoneTransforms[input.boneids.w], input.boneweights.w);
    
	final_world = mul(bone_transform, final_world);
#endif
    
    output.outWorldPos = mul(pos, final_world);
    output.outPosition = mul(output.outWorldPos, vpMatrix);
    output.outNormal = normalize(mul(normal, final_world)).xyz;
#ifdef HAS_TANGENT
    output.outTangent = normalize(mul(tangent, final_world)).xyz;
    output.outBitangent = normalize(mul(bitangent, final_world)).xyz;
#endif
    output.outTexCoord = input.inTexCoord;
    return output;
}