#include "Light.hlsli"
#include "Clustering.hlsli"

cbuffer PSPerCameraBuffer : register(b9)
{
	float scr_w;
	float scr_h;
	float zNear;
	float zFar;
}


cbuffer PSPerFrameBuffer : register(b10)
{
	float4 eyePos;
	float elapsed;
	float delta;
	float2 padding;
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
#if WPS
	float4 worldPos : WPOS;
#endif
#if DEPTH
	float depth : ZDEPTH;
#endif
};

// Samplers
SamplerState Sampler : register(s0);


// TEXTURES BEGIN
#if TEX_DIF
Texture2D diffuseMap : register(t0);
#endif

#if TEX_NRM
Texture2D normalMap : register(t1);
#endif

#if TEX_SPC
Texture2D specularMap : register(t2);
#endif

#if TEX_SHN
Texture2D shininessMap : register(t3);
#endif

#if TEX_OCT
Texture2D opacityMap : register(t4);
#endif

#if TEX_DPM
Texture2D displacementMap : register(t5);
#endif

#if TEX_AOM
Texture2D ambientOcclusionMap : register(t6);
#endif

#if TEX_MTL
Texture2D metallicMap : register(t7);
#endif

// These are not supported on load because they are usually generated
#if TEX_RFL
Texture2D reflectionMap : register(t8);
#endif

#if TEX_RFR
Texture2D refractionMap : register(t9);
#endif
// TEXTURES END

// This must be provided by the material if required
//static const float SpecularPower = 8.f;

void calcColour(in PLight pl, in PixelInputType input, in float3 viewDir, inout float3 lightContrib);


float4 main(PixelInputType input) : SV_TARGET
{
	// Independent of lights, determined once
#ifdef NRM
	input.normal = normalize(input.normal);
#endif

#if TEX > 0
	float4 colour = diffuseMap.Sample(Sampler, input.tex);
#endif
	
#ifdef LIT && WPOS	// lit implies WPOS, fix this
	float3 viewDir = normalize(input.worldPos.xyz - eyePos.xyz);

	// Light code using clustered shading
	float viewDepth = zToViewSpace(input.position.z, zNear, zFar);
	uint clusterIndex = getClusterIndex(trunc(input.position.xy), viewDepth, n, f);

	uint minOffset = offsetGrid[clusterIndex].x;
	uint maxOffset = offsetGrid[clusterIndex + 1].x;

	// Process lights
	float3 lightContrib = float3(0., 0., 0.);

	for (uint i = minOffset; i < maxOffset; i++)
	{
		uint indices = lightIndexBuffer[i];
		PLight pl = lightBuffer[indices];
		calcColour(pl, input, viewDir, lightContrib);
	}

	// fake ambient/directional
	colour.xyz *= max(lightContrib, float3(.1, .1, .1));
#endif

#ifdef GAMMA
	colour.rgb = gammaCorrect(colour.xyz, 1.0f / 2.2f);
#endif

	return colour;
}


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
	float specIntensity = pow(saturate(dot(reflection, viewDir)), 8.f);
	float3 specular = lightColour * specIntensity * diffIntensity;

	lightContrib += ((diffuse + specular) * intensity);
}