#include "Reserved_CB_VS.hlsli"

struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};


struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
	float4 clipSpace : CLIPSPACE;
	float3 tangent : TANGENT;
};


PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	input.position.w = 1.0f;

	output.worldPos = mul(input.position, worldMatrix);
	output.position = mul(output.worldPos, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.clipSpace = output.position;

	output.texCoords = input.tex;

	output.normal = normalize(mul(input.normal, (float3x3)worldMatrix));

	output.tangent = mul(input.tangent, worldMatrix);

	return output;
}