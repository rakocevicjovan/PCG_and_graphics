#include "Reserved_CB_VS.hlsli"

#ifdef SIW
cbuffer TransformsBuffer : register(b1)
{
	float4x4 boneTransforms[144];
};
#endif



struct VertexInputType
{
	float4 position : POSITION;

#if TEX > 0
	float2 tex[TEX] : TEXCOORD0;
#endif
#ifdef NRM
	float3 normal : NORMAL;
#endif
#ifdef COL
	float3 colour : COLOUR;
#endif
#ifdef TAN
	float3 tangent : TANGENT;
#endif
#ifdef BTN
	float3 bitangent : BITANGENT;
#endif
#ifdef SIW
	uint4 boneIDs : B_IDX;
	float4 boneWs : B_WGT;
#endif
#ifdef INS
	matrix insWorldMatrix : WORLDMATRIX;
#endif
};



struct PixelInputType
{
	float4 position : SV_POSITION;
#if TEX > 0
	float2 tex[TEX] : TEXCOORD0;
#endif
#ifdef NRM
	float3 normal : NORMAL;
#endif
#ifdef COL
	float3 colour : COLOUR;
#endif
#ifdef TAN
	float3 tangent : TANGENT;
#endif
#ifdef BTN
	float3 bitangent : BITANGENT;
#endif
#if LMOD > 0
	float4 worldPos : WPOS;
#endif
};



PixelInputType main(VertexInputType input)
{
	PixelInputType output;
	output.position.w = 1.f;

	float4x4 totalMatrix = worldMatrix;

#ifdef INS
	totalMatrix = mul(totalMatrix, insWorldMatrix);		// Might be incorrect
#endif

#ifdef SIW
	float4x4 boneTransform;
	
	boneTransform   = boneTransforms[input.boneIDs.x] * input.boneWs.x;
	boneTransform  += boneTransforms[input.boneIDs.y] * input.boneWs.y;
	boneTransform  += boneTransforms[input.boneIDs.z] * input.boneWs.z;
	boneTransform  += boneTransforms[input.boneIDs.w] * input.boneWs.w;

	totalMatrix = mul(boneTransform, totalMatrix);
#endif

	output.position = mul(input.position, totalMatrix);
#if LMOD > 0
	output.worldPos = output.position;
#endif
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

#if TEX > 0
	output.tex = input.tex;
#endif

float3x3 normalMat = (float3x3)totalMatrix;	//transpose(inverse((float3x3)totalMatrix)) with non-uniform scaling

#ifdef NRM
	output.normal = mul(input.normal, normalMat);		
	output.normal = normalize(output.normal);	// Possibly not necessary
#endif

#ifdef TAN
	output.tangent = mul(input.tangent, normalMat);
#endif

#ifdef BTN
	output.bitangent = mul(input.bitangent, normalMat);
#endif

	return output;
}