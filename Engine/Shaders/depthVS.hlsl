#include "Reserved_CB_VS.hlsli"

struct VertexInputType
{
    float4 position : POSITION;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;	//sv_position creates an offset of 0.5 therefore we need this as well
};



PixelInputType main(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.depthPosition = output.position;
	
    return output;
}