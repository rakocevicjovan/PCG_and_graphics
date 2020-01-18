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

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
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
    float4 wPos : WPOS;
};


PixelInputType main(VertexInputType input)
{
    PixelInputType output;

    output.wPos = mul(input.position, worldMatrix);
    output.position = mul(output.wPos, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    return output;
}