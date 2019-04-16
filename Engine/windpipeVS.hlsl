cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
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


PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;

    output.wPos = mul(input.position, worldMatrix);
    output.position = mul(output.wPos, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    return output;
}