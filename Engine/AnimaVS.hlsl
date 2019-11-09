cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


cbuffer VariableBuffer : register(b1)
{
	float delta;
	float3 padding;
};


cbuffer TransformsBuffer : register(b2)
{
	float4x4 boneTransforms[96];
};


struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	uint4 boneIDs : BONE_ID;
	float4 boneWs : BONE_W;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
};


PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	input.position.w = 1.f;

	float4x4 boneTransform;
	
	boneTransform   = boneTransforms[input.boneIDs.x] * input.boneWs.x;
	boneTransform  += boneTransforms[input.boneIDs.y] * input.boneWs.y;
	boneTransform  += boneTransforms[input.boneIDs.z] * input.boneWs.z;
	boneTransform  += boneTransforms[input.boneIDs.w] * input.boneWs.w;
	

	float4 wat = mul(input.position, boneTransform);
	output.worldPos = mul(wat, worldMatrix);
	output.position = mul(output.worldPos, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.tex = input.tex;
	//output.normal = mul(input.normal, (float3x3)boneTransform);		//transpose(inverse((float3x3)worldMatrix)) with non-uniform scaling
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

	return output;
}