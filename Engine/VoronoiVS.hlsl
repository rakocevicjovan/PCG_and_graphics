struct VertexInputType 
{
	float4 position : POSITION;
};

struct PixelInputType 
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

PixelInputType LightVertexShader(VertexInputType input) {

	input.position.w = 1.0f;

	PixelInputType output;

	output.position = input.position;
	output.tex = float2(input.position.x, -input.position.y);
	return output;
}