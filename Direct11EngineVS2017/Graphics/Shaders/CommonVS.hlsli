inline void FullScreenTriangle(uint id, out float4 pos)
{
    float2 texcoord = float2((id << 1) & 2, id & 2);
    pos = float4(texcoord.x * 2 - 1, -texcoord.y * 2 + 1, 0, 1);
}

inline void FullScreenTriangle(uint id, out float4 pos, out float2 tex)
{
    float2 texcoord = float2((id << 1) & 2, id & 2);
    tex = texcoord;
    pos = float4(texcoord.x * 2 - 1, -texcoord.y * 2 + 1, 0, 1);
}