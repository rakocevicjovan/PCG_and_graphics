cbuffer MatrixBuffer {
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


cbuffer VariableBuffer {
	float delta;
	float3 padding;
};


struct VertexInputType {
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
	float time : MYTIME;
};


PixelInputType LightVertexShader(VertexInputType input) {

	PixelInputType output;

	float4 worldPos = mul(input.position, worldMatrix);

	output.worldPos = worldPos;	//careful... doing this to optimize and avoid copying
	output.position = mul(worldPos, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.tex = input.tex;
	output.normal = mul(input.normal, (float3x3)worldMatrix);		//transpose(inverse((float3x3)worldMatrix)) with non-uniform scaling
	output.normal = normalize(output.normal);

	output.time = delta;

	return output;
}