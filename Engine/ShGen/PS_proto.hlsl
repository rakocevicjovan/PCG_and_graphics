#include "Light.hlsli"
#include "Clustering.hlsli"

cbuffer PSPerFrameBuffer : register(b10)
{
	float4 eyePos;
	float elapsed;
	float delta;
	float2 padding;
}

cbuffer PSPerCameraBuffer : register(b11)
{

}

struct PLight
{
	float4 rgbi;
	float4 posRange;
};

// Clustered shading buffers
StructuredBuffer<PLight> lightBuffer : register(t15);
StructuredBuffer<uint> lightIndexBuffer : register(t16);
StructuredBuffer<uint2> offsetGrid : register(t17);

struct PixelInputType
{
	float4 position : SV_POSITION;
#if TEX > 0
	float2 tex[TEX] : TEXCOORD0;
#endif
#if NRM
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
#if WPS
	float4 worldPos : WPOS;
#endif
#if DEPTH
	float depth : ZDEPTH;
#endif
};

// Samplers
SamplerState SampleType;

// Textures
Texture2D shaderTexture : register(t0);

static const float SpecularPower = 8.f;

float4 main(PixelInputType input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	// Don't hardcode this, provide in a buffer
	float n = 1.;
	float f = 1000.;
	float viewDepth = zToViewSpace(input.position.z, n, f);
	uint clusterIndex = getClusterIndex(trunc(input.position.xy), viewDepth, n, f);
	//uint3 xyz = getClusterIndexTriplet(input.position.xy, viewDepth, n, f);

	// Independent of lights, determined once
	float4 colour = shaderTexture.Sample(SampleType, input.tex);	// texture colour
	float3 viewDir = normalize(input.worldPos.xyz - eyePos.xyz);	// view vector

	uint minOffset;
	uint maxOffset;

	minOffset = offsetGrid[clusterIndex].x;
	maxOffset = offsetGrid[clusterIndex + 1].x;

	// Process lights
	float3 lightContrib = float3(0., 0., 0.);

	PLight pl;


	for (uint i = minOffset; i < maxOffset; i++)
	{
		uint indices = lightIndexBuffer[i];
		pl = lightBuffer[indices];
		calcColour(pl, input, viewDir, lightContrib);

		// When I fix the artefact and get back to this, try using 16 bit indices
		//uint firstIndex  = indices >> 16;				// get top half, shift bottom half to oblivion
		//pl = lightBuffer[firstIndex];
		//calcColour(pl, input, viewDir, lightContrib);

		//uint secondIndex = (indices & 0x0000FFFF);	// mask out top half, get bottom half
		//pl = lightBuffer[secondIndex];
		//calcColour(pl, input, viewDir, lightContrib);

		//lightContrib = (float3)(span);
	}

	//colour.xyz = (float3)((maxOffset - minOffset) / 3.);

	colour.xyz = max(lightContrib, float3(1., 1., 1.) * 0.1);	// fake ambient/directional		//  * colour.xyz
	//colour.xyz = (lightContrib + float3(1., 1., 1.) * 0.1);

	//colour.rgb = gammaCorrect(colour.xyz, 1.0f / 2.2f);

	return colour;
}