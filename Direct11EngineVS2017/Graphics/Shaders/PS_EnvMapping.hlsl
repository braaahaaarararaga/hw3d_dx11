/* NOTE  TODO

void main(....)
{
    float3 eyev = inWorldPosition.xyz - CameraPosition.xyz;
    eyev = normalize(eyev);
    float3 refv = reflect(eye.xyz, inNormal.xyz);
    refv = normalize(refv);
    float2 envTexCoord;
    envTexCoord.x = -refv.x * 0.3 + 0.5;
    envTexCoord.y = -refv.y * 0.3 + 0.5;
    outDiffuse = g_Texture.sampler(... , envTexCoord);
}
*/ 