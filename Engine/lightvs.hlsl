#include "Reserved_CB_VS.hlsli"

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
	float4 worldPos : WPOS;
};


PixelInputType main(VertexInputType input)
{
	PixelInputType output;

    output.worldPos = mul(input.position, worldMatrix);
    output.position = mul(output.worldPos, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	output.tex = input.tex;

    output.normal = mul(input.normal, (float3x3)worldMatrix);		//transpose(inverse((float3x3)worldMatrix)) with non-uniform scaling
    output.normal = normalize(output.normal);

    return output;
}