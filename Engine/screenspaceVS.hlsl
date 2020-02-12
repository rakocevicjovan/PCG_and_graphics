struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};


PixelInputType main(VertexInputType input)
{
	input.position.w = 1.0f;

	PixelInputType output;

	output.position = input.position;

	output.tex = float2(input.tex.x, 1.f - input.tex.y);

	return output;
}