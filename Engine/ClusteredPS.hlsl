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
StructuredBuffer<LightIndex> lightIndexBuffer : register(t16);
StructuredBuffer<OffsetCount> offsetGrid : register(t17);




//go 2-4 times higher on this when using blinn phong compared to phong, should be material defined
static const float SpecularPower = 8.f;

float4 main(PixelInputType input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float n = 1.;
	float f = 1000.;

	float depth = zToViewSpace(input.position.z, n, f);

	uint clusterIndex = getClusterIndex(input.position.xy, depth, n, f);	// Don't hardcode this, provide in a buffer

	uint minIndex = offsetGrid[clusterIndex].oc;
	uint maxIndex = offsetGrid[clusterIndex + 1].oc;

	// independent of lights, determined once
	
	float4 colour = shaderTexture.Sample(SampleType, input.tex);	// texture colour
	float3 viewDir = normalize(input.worldPos.xyz - eyePos.xyz);	// view vector

	float3 lightContrib = float3(0., 0., 0.);

	for (uint i = minIndex; i < maxIndex; i++)
	{
		lightContrib += .5f;
		
		/*
		PLight pl = lightBuffer[lightIndexBuffer[i].getIndex()];
		float3 lightColour = pl.rgbi.xyz;

		float3 lightDir = normalize(input.worldPos.xyz - pl.posRange.xyz);
		float3 invLightDir = -lightDir;

		//calculate ambient light
		float4 ambient = calcAmbient(lightColour, 1.);

		//calculate diffuse light
		float diffIntensity = 0.f;
		float4 diffuse = calcDiffuse(invLightDir, input.normal, lightColour, 1., diffIntensity);

		//calculate specular light
		float4 specular = calcSpecularPhong(invLightDir, input.normal, lightColour, 1., viewDir, diffIntensity, SpecularPower);

		lightContrib += (ambient.xyz + diffuse.xyz + specular.xyz);
		*/
	}

	colour.xyz = lightContrib * colour.xyz;

	// Cluster determination works
	//colour.xyz = (float3) (getClusterIndexTriplet(input.position.xy, depth, n, f) / float3(30., 17., 16.));

	//colour.rgb = gammaCorrect(colour.xyz, 1.0f / 2.2f);

	return colour;
}