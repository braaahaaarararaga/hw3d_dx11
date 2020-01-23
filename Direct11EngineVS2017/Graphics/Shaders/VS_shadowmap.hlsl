#pragma pack_matrix( row_major )

cbuffer perObjectBuffer : register(b0)
{
    float4x4 vpMatrix;
    float4x4 worldMatrix;
};


struct VS_INPUT
{
    float3 inPos : POSITION;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.outPosition = mul(mul(float4(input.inPos, 1.0f), worldMatrix), vpMatrix);
    return output;
}