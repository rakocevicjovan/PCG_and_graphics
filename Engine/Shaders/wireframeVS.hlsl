#include "Reserved_CB_VS.hlsli"

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD;
	float3 normal : NORMAL;
};

struct GeomInputType
{
    float4 position : SV_POSITION;
	float4 worldPos : WPOS;
};

GeomInputType main(VertexInputType input)
{
   	GeomInputType output;

    output.worldPos = mul(input.position, worldMatrix);
    output.position = mul(output.worldPos, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    return output;
}