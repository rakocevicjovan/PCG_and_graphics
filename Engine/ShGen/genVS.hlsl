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
	matrix worldMatrix : WORLDMATRIX;
#endif
};

struct PixelInputType
{
	float4 position : SV_POSITION;

#ifdef OTEX
	float2 tex : TEXCOORD0;
#endif

#ifdef ONRM
	float3 normal : NORMAL;
#endif

#ifdef OWPS
	float3 worldPos : WPOS;
#endif

#ifdef OTAN
	float3 tangent : WPOS;
#endif
};


PixelInputType main(VertexInputType input)
{
	PixelInputType output;

#ifdef OWPS
	output.worldPos = mul(input.position, worldMatrix);
	output.position = mul(output.worldPos, viewMatrix);
#else
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
#endif

	output.position = mul(output.position, projectionMatrix);

#ifdef OTEX
	output.tex = input.tex;
#endif

#ifdef ONRM
	output.normal = mul(input.normal, (float3x3)worldMatrix);		//transpose(inverse((float3x3)worldMatrix)) with non-uniform scaling
	output.normal = normalize(output.normal);
#endif

#ifdef OTAN
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
#endif

	return output;
}