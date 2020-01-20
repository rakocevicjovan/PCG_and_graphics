cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
};

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
};


PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	input.position.w = 1.;

	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

    output.tex = float2(input.tex.x, input.tex.y);

	return output;
}