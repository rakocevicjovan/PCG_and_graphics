cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


cbuffer VariableBuffer
{
	float delta;
	float3 padding;
};


struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	matrix worldMatrix : WORLDMATRIX;
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

    float4x4 totalMatrix = mul(input.worldMatrix, worldMatrix);
	output.worldPos = mul(input.position, totalMatrix);
	output.position = mul(output.worldPos, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.tex = input.tex;
    output.normal = mul(input.normal, (float3x3) totalMatrix);
	output.normal = normalize(output.normal);

	return output;
}