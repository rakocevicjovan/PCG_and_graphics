#include "Light.hlsli"

cbuffer LightBuffer : register(b0)
{
	float3 alc;
	float ali;
	float3 dlc;
	float dli;
	float3 slc;
	float sli;
	float4 lightPosition;
	float4 stupidThingIUsedToDo;
};

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
	float depth : ZDEPTH;
};


cbuffer ShadowBuffer : register(b11)
{
	matrix lvpMatrix[NUM_CASCADES];
	float4 cascadeLimits;			//rudimentary but ok for now, I will hardly need more than 4 cascades anyways
}

Texture2D shaderTexture : register(t0);
Texture2DArray<float> csms : register(t11);

SamplerState Sampler;

//go 2-4 times higher on this when using blinn phong compared to phong, should be material defined
static const float SpecularPower = 8.f;



float4 main(PixelInputType input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);
	float3 invLightDir = -lightDir;
	float3 viewDir = normalize(input.worldPos.xyz - eyePos.xyz);

	float percentageLit = obscur(input.depth, XYDIM, lvpMatrix, cascadeLimits, input.worldPos, csms, Sampler);

	//calculate ambient light
	float4 ambient = calcAmbient(alc, ali);

	//calculate diffuse light
	float diffIntensity = 0.f;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, dlc, dli, diffIntensity);

	//calculate specular light
	float4 specular = calcSpecularPhong(invLightDir, input.normal, slc, sli, viewDir, diffIntensity, SpecularPower);

	float4 colour = shaderTexture.Sample(Sampler, input.tex);
	colour = (ambient + diffuse * percentageLit) * colour + specular * percentageLit;	// Incorporate the "lit-ness" into the final calculation

	//apply gamma correction
	colour.rgb = gammaCorrect(colour.xyz, 1.f / 2.2f);

	colour.a = 1.f;

	return colour;
}


/* Old single sample method, way too blocky */

// Sample version
//float2 shadowCoord2 = float2(shadowCoord.x / shadowCoord.w / 2.0f + 0.5f, -shadowCoord.y / shadowCoord.w / 2.0f + 0.5f);
//float closestDepth = csms.Sample(Sampler, float3(shadowCoord2.x, shadowCoord2.y, index)).x;

// Load version
//float closestDepth = csms.Load(float4(shadowCoord2.x * XYDIM, shadowCoord2.y * XYDIM, index, 0));