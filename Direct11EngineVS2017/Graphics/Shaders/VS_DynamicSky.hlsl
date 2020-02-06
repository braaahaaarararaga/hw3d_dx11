struct VS_INPUT
{
    uint id : SV_VertexID;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 clipspace : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    float2 texcoord = float2((input.id << 1) & 2, input.id & 2);
    output.position = float4(texcoord.x * 2 - 1, -texcoord.y * 2 + 1, 0, 1);
    
    output.position = output.position.xyww;
    output.clipspace = output.position.xy;
	
    return output;
}