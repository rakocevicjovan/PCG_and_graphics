#include "Reserved_CB_VS.hlsli"

cbuffer LVPBuffer: register(b1)
{
	matrix lvpMatrix;
};

struct VertexInputType
{
	float4 position : POSITION;
};



float4 main(VertexInputType input) : SV_POSITION
{
	input.position.w = 1.0f;

	float4 CSPOS = mul(input.position, worldMatrix);
	CSPOS = mul(CSPOS, lvpMatrix);

	return CSPOS;
}



/*
	struct VertexOutputType
	{
		float depth : DEPTH;
	};

	//VertexOutputType output;
	output.depth = CSPOS.z / CSPOS.w;
	return output;
*/