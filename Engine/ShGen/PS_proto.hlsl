#include "Light.hlsli"
#include "Clustering.hlsli"
#include "Reserved_CB_PS.hlsli"

cbuffer LightBuffer : register(b0)
{
	float3 alc;
	float ali;
	float3 dlc;
	float dli;
	float3 slc;
	float sli;
	float4 lightPosition;
};

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
#if LMOD > 0
	float4 worldPos : WPOS;
#endif
};

// Samplers
SamplerState Sampler : register(s0);

// Once I implement this, instead of fixed registry indices i can use these
// #define T(a) t##a
// T(TEX_DIF_REG) 
// Also, the index of texture coordinates used by each texture will be defined as UVCH_<TYPE>

// TEXTURES BEGIN
#if TEX_DIF > 0
Texture2D diffuseMap : register(t0);
#endif
#if TEX_NRM > 0
Texture2D normalMap : register(t1);
#endif
#if TEX_SPC > 0
Texture2D specularMap : register(t2);
#endif
#if TEX_SHN > 0
Texture2D shininessMap : register(t3);
#endif
#if TEX_OPC > 0
Texture2D opacityMap : register(t4);
#endif
#if TEX_DPM > 0
Texture2D displacementMap : register(t5);
#endif
#if TEX_AOM > 0
Texture2D ambientOcclusionMap : register(t6);
#endif
#if TEX_MTL > 0
Texture2D metallicMap : register(t7);
#endif
#if TEX_RGH > 0
Texture2D roughnessMap : register(t8);
#endif

// These are may be generated
#if TEX_RFL > 0
Texture2D reflectionMap : register(t9);
#endif

#if TEX_RFR > 0
Texture2D refractionMap : register(t10);
#endif
// TEXTURES END

void calcColour(in PLight pl, in PixelInputType input, in float3 viewDir, inout float3 lightContrib);


float4 main(PixelInputType input) : SV_TARGET
{
#if NRM > 0
	input.normal = normalize(input.normal);	// Might not be needed before map
// Does NOT use passed in bitangents yet, ignores them @TODO
#if TEX_NRM > 0 && TAN > 0 && TEX > 0	// Dependencies are already resolved tbh
	mapNormals(Sampler, normalMap, input.tex[0], input.tangent, input.normal);
#endif
#endif

	float4 colour = (float4)(1.);

#if TEX_DIF > 0
	colour = diffuseMap.Sample(Sampler, input.tex[0]);
#endif

#if TEX_OPC > 0
	//colour.w = opacityMap.Sample(Sampler, input.tex[0]).r;
#endif

	//if (colour.w < 0.000001f) discard;

#if COLOUR > 0
	colour.xyz = input.colour;
#endif
	
#ifdef (LMOD > 0)
	float3 viewDir = input.worldPos.xyz - eyePos.xyz;
	float distToPoint = length(viewDir);
	viewDir /= distToPoint;


	// Light code using clustered shading
	/*
	float viewDepth = zToViewSpace(input.position.z, zNear, zFar);
	uint clusterIndex = getClusterIndex(trunc(input.position.xy), viewDepth, zNear, zFar);

	uint minOffset = offsetGrid[clusterIndex].x;
	uint maxOffset = offsetGrid[clusterIndex + 1].x;


	float3 lightContrib = float3(0., 0., 0.);

	for (uint i = minOffset; i < maxOffset; i++)
	{
		uint indices = lightIndexBuffer[i];
		PLight pl = lightBuffer[indices];
		calcColour(pl, input, viewDir, lightContrib);
	}
	
	// fake ambient/directional
	colour.xyz *= max(lightContrib, float3(.1, .1, .1));
	*/

	// Usual way
	float4 ambient = calcAmbient(alc, ali);

	//calculate diffuse light
	float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);

	float diffIntensity = 0.f;
	float4 diffuse = calcDiffuse(-lightDir, input.normal, dlc, dli, diffIntensity);

	//calculate specular light
	float4 specular = calcSpecularPhong(-lightDir, input.normal, slc, sli, viewDir, diffIntensity, 8.f);
#if TEX_SPC > 0
	specular.rgb *= specularMap.Sample(Sampler, input.tex[0]).rgb;
#endif
	colour = (ambient + diffuse + specular) * colour;	// + specular

#endif

#ifdef FOG
	// My current fog function is not general purpose, make one and add
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

	//calculate specular light (none for lambert materials)
#if LMOD > 1
	float3 reflection = normalize(reflect(-lightDir, input.normal));

	float3 specularPower;
#if TEX_SPC > 0
	specularPower = specularMap.Sample(Sampler, input.tex[0]).rgb;
#else
	specularPower = (float3)(1.f);	// Make this a possible param
#endif

	float shininess;
#if TEX_SHN > 0
	shininess = shininessMap.Sample(Sampler, input.tex[0]).r;
#else
	shininess = 8.f;
#endif

	// Could be dot reflection light dir
	float specIntensity = pow(saturate(dot(reflection, viewDir)), shininess);
	float3 specular = lightColour * specIntensity * specularPower;
	//specular *= diffIntensity; Is this needed?

	diffuse += specular;
#endif;

	lightContrib += (diffuse * intensity);
}