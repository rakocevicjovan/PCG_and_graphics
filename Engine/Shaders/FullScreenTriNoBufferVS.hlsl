#include "Reserved_CB_VS.hlsli"

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float3 dir : direction;
};

// Shader assumes top left is uv = (0, 0) and clockwise = front face
// Input vertexId:       0  1  2
// Resulting position:  -1  3 -1
// Resulting UV:         1  1 -3

VSOut main(uint vertexId : SV_VertexID)
{
    VSOut output;

    int x = (vertexId & 1) * 4 - 1;
    int y = 1 - (vertexId & 2) * 2;

    output.pos = float4((float)x, (float)y, 1.0f, 1.0f);
    output.uv = float2((vertexId & 1) * 2, (vertexId & 2));
    
    float4 unprojectedDir = mul(output.pos, invProjectionMatrix); 
    output.dir = mul(unprojectedDir.xyz, (float3x3) cameraMatrix);

    return output;
}