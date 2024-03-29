#include "Reserved_CB_VS.hlsli"

cbuffer ProjectionBuffer : register(b1)
{
	matrix viewMatrix2;
	matrix projectionMatrix2;
};


struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 viewPosition : TEXCOORD1;
};



PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

	float4 wPos = mul(input.position, worldMatrix);

    output.position = mul(wPos, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.viewPosition = mul(wPos, viewMatrix2);
    output.viewPosition = mul(output.viewPosition, projectionMatrix2);

    output.tex = input.tex;
	
	output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

    return output;
}