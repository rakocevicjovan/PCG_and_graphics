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
cbuffer ShadowBuffer : register(b11)
{
	matrix lvpMatrix[NUM_CASCADES];
	float4 cascadeLimits;			//rudimentary but ok for now, I will hardly need more than 4 cascades anyways
}

Texture2D shaderTexture : register(t0);
Texture2DArray<float> csms : register(t10);

SamplerState SampleType;

//go 2-4 times higher on this when using blinn phong compared to phong, should be material defined
static const float SpecularPower = 8.f;

float4 main(PixelInputType input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);
	float3 invLightDir = -lightDir;

	float3 viewDir = normalize(input.worldPos.xyz - eyePos.xyz);

	//texture colour
	float4 colour = shaderTexture.Sample(SampleType, input.tex);

	//calculate ambient light
	float4 ambient = calcAmbient(alc, ali);



	// Determine whether the pixel is shadowed or not	

	//Check how far it is from the camera compared to cascade far planes
	float4 fComparison = (input.depth > cascadeLimits[0]);

	// Determine which cascade it is in, up to NUM_CASCADES
	float fIndex = dot(float4(NUM_CASCADES > 0, NUM_CASCADES > 1, NUM_CASCADES > 2, NUM_CASCADES > 3), fComparison);
	fIndex = min(fIndex, NUM_CASCADES);
	int index = (int)fIndex;

	// Using the selected cascade's light view projection matrix, determine the pixel's position in light space
	float4 shadowCoord = mul(input.worldPos, lvpMatrix[index]);

	// Using the selected cascade's shadow map, determine the depth of the closest pixel to the light along the light direction ray
	float closestDepth = csms.Sample(SampleType, float3(shadowCoord.xy, index));

	// Compare the two - only the pixels closest to the light will be directly illuminated
	// step: 1 if the x parameter is greater than or equal to the y parameter; otherwise, 0.
	float lit = step(closestDepth, shadowCoord.z);	// MIGHT WANT TO MAX(lit, minLight) THIS TO AVOID DARK SHADOWS

	//calculate diffuse light
	float diffIntensity = 0.f;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, dlc, dli, diffIntensity);

	//calculate specular light
	float4 specular = calcSpecularPhong(invLightDir, input.normal, slc, sli, viewDir, diffIntensity, SpecularPower);

	colour = (ambient + diffuse * lit) * colour + specular * lit;	// Incorporate the "lit-ness" into the final calculation

	//apply gamma correction
	colour.rgb = gammaCorrect(colour.xyz, 1.f / 2.2f);

	colour.a = 1.f;
	return colour;
}