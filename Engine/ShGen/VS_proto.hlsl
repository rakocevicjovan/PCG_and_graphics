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
	uint4 boneIDs : BONE_ID;
	float4 boneWs : BONE_W;
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
#ifdef WPS
	float3 worldPos : WPOS;
#endif
};



PixelInputType main(VertexInputType input)
{
	PixelInputType output;

#ifdef INS
	worldMatrix = mul(worldMatrix, insWorldMatrix);		// Might be incorrect
#endif

#ifdef SIW
	float4x4 boneTransform;
	
	boneTransform   = boneTransforms[input.boneIDs.x] * input.boneWs.x;
	boneTransform  += boneTransforms[input.boneIDs.y] * input.boneWs.y;
	boneTransform  += boneTransforms[input.boneIDs.z] * input.boneWs.z;
	boneTransform  += boneTransforms[input.boneIDs.w] * input.boneWs.w;

	worldMatrix = mul(boneTransform, worldMatrix);
#endif

	output.position = mul(input.position, worldMatrix);
#ifdef WPS
	output.worldPos = output.position;
#endif
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

#if TEX > 0
	output.tex = input.tex;
#endif

float3x3 normalMat = (float3x3)worldMatrix;	//transpose(inverse((float3x3)worldMatrix)) with non-uniform scaling

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