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

cbuffer WMBuffer: register(b0)
{
	matrix worldMatrix;
};

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

	//matrix LMVPMatrix = mul(worldMatrix, lvpMatrix);	//careful with ordering

	float4 CSPOS = mul(input.position, worldMatrix);
	CSPOS = mul(CSPOS, lvpMatrix);
	
	//float4 VSPOS = mul(WSPOS, viewMatrix);
	//float4 CSPOS = mul(VSPOS, projectionMatrix);

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