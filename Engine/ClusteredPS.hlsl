#include "Light.hlsli"
#include "Clustering.hlsli"


cbuffer PSPerFrameBuffer : register(b10)
{
	float4 eyePos;
	float elapsed;
	float delta;
	float2 padding;
}


struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
};

Texture2D shaderTexture : register(t0);
SamplerState SampleType;

StructuredBuffer<PLight> lightBuffer : register(t15);
StructuredBuffer<uint> lightIndexBuffer : register(t16);		// must be decomposed to two uint16_t per each
StructuredBuffer<uint> offsetGrid : register(t17);				// can be viewed as uint

static const float SpecularPower = 8.f;



void calcColour(in PLight pl, in PixelInputType input, in float3 viewDir, inout float3 lightContrib)
{
	float3 lightColour = pl.rgbi.xyz;

	float3 lightDir = input.worldPos.xyz - pl.posRange.xyz;
	float dist = length(lightDir);
	lightDir /= dist;

	float3 invLightDir = -lightDir;

	//calculate ambient light
	float4 ambient = calcAmbient(lightColour, 1.);

	//calculate diffuse light
	float diffIntensity = 0.f;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, lightColour, 1., diffIntensity);

	//calculate specular light
	float4 specular = calcSpecularPhong(invLightDir, input.normal, lightColour, 1., viewDir, diffIntensity, SpecularPower);

	lightContrib += (ambient.xyz + diffuse.xyz + specular.xyz) / (dist * dist);
}



float4 main(PixelInputType input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	// Don't hardcode this, provide in a buffer
	float n = 1.;
	float f = 1000.;
	float depth = zToViewSpace(input.position.z, n, f);
	uint clusterIndex = getClusterIndex(input.position.xy, depth, n, f);
	//uint3 xyz = getClusterIndexTriplet(input.position.xy, depth, n, f);

	// Independent of lights, determined once
	float4 colour = shaderTexture.Sample(SampleType, input.tex);	// texture colour
	float3 viewDir = normalize(input.worldPos.xyz - eyePos.xyz);	// view vector

	uint minOffset;
	uint maxOffset;

	//minOffset = offsetGrid[clusterIndex];
	//maxOffset = offsetGrid[clusterIndex + 1];

	// Span of indices belonging to the cluster
	// List index list is in 4 byte chunks, but each value packed is 2 bytes, therefore we address the list at clusterIndex/2
	
	minOffset = ((offsetGrid[clusterIndex]) >> 16);
	minOffset = (minOffset / 2u);	// + (minOffset % 2)

	maxOffset = ((offsetGrid[clusterIndex + 1]) >> 16);
	maxOffset = (maxOffset / 2u) + 2;
	

	uint span = maxOffset - minOffset;

	

	// Process lights
	float3 lightContrib = float3(0., 0., 0.);

	for (uint i = minOffset; i < maxOffset; i++)
	{
		uint twoIndex = lightIndexBuffer[i];

		uint firstIndex  = twoIndex >> 16;			// get top half, shift bottom half to oblivion
		uint secondIndex = twoIndex & 0x0000FFFF;	// mask out top half, get bottom half
		
		PLight pl = lightBuffer[twoIndex];
		calcColour(pl, input, viewDir, lightContrib);
		
		pl = lightBuffer[secondIndex];
		calcColour(pl, input, viewDir, lightContrib);

		//lightContrib = (float3)(span);
	}

	//colour.xyz = lightContrib;// *colour.xyz;

	// Cluster determination works
	//colour.xyz = (float3) (getClusterIndexTriplet(input.position.xy, depth, n, f) / float3(GX, GY, GZ));
	//colour.xyz = (float3) (gcit(input.position.xy, depth, n, f) / float3(GX, GY, GZ));

	//colour.rgb = gammaCorrect(colour.xyz, 1.0f / 2.2f);

	return colour;
}