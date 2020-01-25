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

cbuffer ProjectionBuffer : register(b1)
{
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
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
	float4 worldPosition : TEXTURE;
    float4 fragPosLightSpace : TEXCOORD1;
};



PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

	float4 wPos = mul(input.position, worldMatrix);

    output.worldPosition = wPos;
    output.position = mul(output.worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Calculate the position of the vertices as viewed by the light source.
    output.fragPosLightSpace = mul(wPos, lightViewMatrix);
    output.fragPosLightSpace = mul(output.fragPosLightSpace, lightProjectionMatrix);

    output.tex = input.tex;

    output.normal = normalize(mul(input.normal, (float3x3)worldMatrix));

    return output;
}