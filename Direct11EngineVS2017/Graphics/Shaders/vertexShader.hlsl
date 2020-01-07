#pragma pack_matrix( row_major )

cbuffer perObjectBuffer : register(b0)
{
    float4x4 vpMatrix;
    float4x4 worldMatrix;
};


struct VS_INPUT
{
    float3 inPos : POSITION;
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
    
    
    output.outWorldPos = mul(pos, worldMatrix);
    output.outPosition = mul(output.outWorldPos, vpMatrix);
    output.outNormal = normalize(mul(normal, worldMatrix)).xyz;
    output.outTangent = normalize(mul(tangent, worldMatrix)).xyz;
    output.outBitangent = normalize(mul(bitangent, worldMatrix)).xyz;
    output.outTexCoord = input.inTexCoord;
    return output;
}