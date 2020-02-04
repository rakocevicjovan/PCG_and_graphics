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



struct VertexInputType
{
	float4 position : POSITION;

#ifdef TEX
	float2 tex : TEXCOORD0;
#endif

#ifdef NRM
	float3 normal : NORMAL;
#endif

#ifdef TAN
	float3 tangent : TANGENT;
#endif

#ifdef INS
	matrix insWorldMatrix : WORLDMATRIX;
#endif
};



struct PixelInputType
{
	float4 position : SV_POSITION;

#ifdef TEX
	float2 tex : TEXCOORD0;
#endif

#ifdef NRM
	float3 normal : NORMAL;
#endif

#ifdef WPS
	float3 worldPos : WPOS;
#endif

#ifdef TAN
	float3 tangent : TANGENT;
#endif
};



PixelInputType main(VertexInputType input)
{
	PixelInputType output;

#ifdef INS
	worldMatrix = mul(worldMatrix, insWorldMatrix);		// don't think this is correct...
#endif

#ifdef WPS
	output.worldPos = mul(input.position, worldMatrix);
	output.position = mul(output.worldPos, viewMatrix);
#else
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
#endif

	output.position = mul(output.position, projectionMatrix);

#ifdef TEX
	output.tex = input.tex;
#endif

#ifdef NRM
	output.normal = mul(input.normal, (float3x3)worldMatrix);		//transpose(inverse((float3x3)worldMatrix)) with non-uniform scaling
	output.normal = normalize(output.normal);
#endif

#ifdef TAN
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
#endif

	return output;
}