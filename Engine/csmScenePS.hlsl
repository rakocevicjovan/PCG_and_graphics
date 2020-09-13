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


#define NUM_CASCADES 3
#define XYDIM 1024.

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

	// Determine whether the pixel is shadowed or not
	// Check how far it is from the camera compared to cascade far planes
	float4 fComparison = float4(input.depth > cascadeLimits[0], input.depth > cascadeLimits[1], input.depth > cascadeLimits[2], input.depth > cascadeLimits[3]);

	// Determine which cascade it is in, up to NUM_CASCADES
	float fIndex = dot(float4(NUM_CASCADES > 0, NUM_CASCADES > 1, NUM_CASCADES > 2, NUM_CASCADES > 3), fComparison);
	int index = (int)(min(fIndex, NUM_CASCADES - 1));

	// Using the selected cascade's light view projection matrix, determine the pixel's position in light space
	float4 shadowCoord = mul(input.worldPos, lvpMatrix[index]);
	
	float2 shadowCoord2 = float2(shadowCoord.x / shadowCoord.w / 2.0f + 0.5f, -shadowCoord.y / shadowCoord.w / 2.0f + 0.5f);
	// Using the selected cascade's shadow map, determine the depth of the closest pixel to the light along the light direction ray

	float4 shadowMapDists = (float4)(0.f);
	float percentageLit = 0.f;
	
	// pcf
	for (float i = 0; i < 1.99; i += 1.)
	{
		for (float j = 0; j < 1.99; j += 1.)
		{
			
			shadowMapDists[i + j] += csms.Sample(Sampler, float3(
				(shadowCoord.x -  (.5 + i) / XYDIM) / shadowCoord.w / 2.0f + 0.5f,
				(-shadowCoord.y - (.5 + j) / XYDIM) / shadowCoord.w / 2.0f + 0.5f,
				(float) index)).x;
			
			/*
			shadowMapDists += csms.Load(int4(
											shadowCoord2.x * XYDIM. - 2 + i * 4,
											shadowCoord2.y * XYDIM. - 2 + j * 4,
											index, 
											0)).x;
			*/
			
			// Works but a single dot product is faster
			//percentageLit += step(shadowCoord.z, shadowMapDists[i+j] + 0.0001) * .25f;
		}
	}

	percentageLit = dot((shadowCoord.z < shadowMapDists + 0.00001), float4(.25, .25, .25, .25));
	//percentageLit = smoothstep(0., 1., percentageLit);

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
//float closestDepth = csms.Sample(Sampler, float3(shadowCoord2.x, shadowCoord2.y, index)).x;

// Load version
//float closestDepth = csms.Load(float4(shadowCoord2.x * XYDIM, shadowCoord2.y * XYDIM, index, 0));