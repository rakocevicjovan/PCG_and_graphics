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
	float3 texCoord : TEXCOORD;
};

PixelInputType CMVS(VertexInputType input) {

	PixelInputType output;

	output.position = mul(input.position, worldMatrix);	//careful... doing this to optimize and avoid copying
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix).xyww;

	output.texCoord = input.position.xyz;

	return output;
}