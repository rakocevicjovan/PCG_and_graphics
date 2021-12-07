cbuffer PerCameraBuffer : register(b10)
{
	matrix projectionMatrix;
};

cbuffer PerFrameBuffer : register(b11)
{
	matrix viewMatrix;
	float dTime;
	float eTime;
	float2 padding;
};

cbuffer WMBuffer: register(b0)
{
	matrix worldMatrix;
};

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