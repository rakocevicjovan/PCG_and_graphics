#include "Light.hlsli"
#include "Clustering.hlsli"
#include "Reserved_CB_PS.hlsli"

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
};

Texture2D shaderTexture : register(t0);
SamplerState SampleType;
static const float SpecularPower = 8.f;

StructuredBuffer<PLight> lightBuffer : register(t15);
StructuredBuffer<uint> lightIndexBuffer : register(t16);
StructuredBuffer<uint2> offsetGrid : register(t17);

void calcColour(in PLight pl, in PixelInputType input, in float3 viewDir, inout float3 lightContrib)
{
	float3 lightColour = pl.rgbi.rgb;

	float3 lightDir = input.worldPos.xyz - pl.posRange.xyz;
	float dist = length(lightDir);
	lightDir /= dist;

	float intensity = pl.rgbi.w / (dist * dist);	// Square fallof, apparently correct but a bit aggressive maybe?

	//calculate diffuse light
	float diffIntensity = max(dot(input.normal, -lightDir), 0.0f);
	float3 diffuse = lightColour * diffIntensity;

	//calculate specular light
	float3 reflection = normalize(reflect(-lightDir, input.normal));
	float specIntensity = pow(saturate(dot(reflection, viewDir)), SpecularPower);
	float3 specular = lightColour * specIntensity * diffIntensity;

	lightContrib += ((diffuse + specular) * intensity);
}



float4 main(PixelInputType input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	// Don't hardcode this, provide in a buffer
	float viewDepth = zToViewSpace(input.position.z, zNear, zFar);
	uint clusterIndex = getClusterIndex(trunc(input.position.xy), viewDepth, zNear, zFar);
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