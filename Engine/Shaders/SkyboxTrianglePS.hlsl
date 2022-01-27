#include "Reserved_CB_PS.hlsli"

TextureCube cubeMapTexture : register(t0);
SamplerState Sampler : register(s0);

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 dir : direction;
};

float4 main(PixelInputType input) : SV_TARGET
{
    // Input texture coordinates are in 0-1 range, (0, 0) being top left
    //float4 vecToPixel = float4(float2(input.texCoord.x, 1. - input.texCoord.y) * 2. - 1., 1.0f, 1.f);

    // Create and unproject the ray - I do this in the vertex shader now due to ridiculously lower cost
    //float4 vecToPixel = float4(float2(input.texCoord.x, 1. - input.texCoord.y) * 2. - 1., 1., 1.);
    //vecToPixel = normalize(mul(vecToPixel, invProjectionMatrix));
    //vecToPixel.xyz = normalize(mul(vecToPixel.xyz, (float3x3) cameraMatrix));

    return cubeMapTexture.Sample(Sampler, input.dir);
}