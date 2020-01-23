#include "Noise.hlsli"

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

cbuffer VariableBuffer : register(b1)
{
	float elapsed;
	float3 padding;
};


cbuffer ViewDirBuffer : register(b2)
{
	float4 eyePos;
};



struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 msPos : MSPOS;
	float4 wPos : WPOS;
};

Texture2D shaderTexture;
SamplerState SampleType;


static const float LACUNARITY = 1.9357f;
static const float GAIN = 0.5317f;;
static const int NUM_OCTAVES = 5;

static const int NUM_STEPS = 30;
static const float STEP_SIZE = 2.0f / (float)NUM_STEPS;


float4 raymarch(in float3 rayOrigin, in float3 rayDir)
{
	//initialize the sum of colours we will get in the end
	float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float density = 0.f;
	float t = 0.f;

	float3 sampleCoords = rayOrigin * 3.33f;

	float warper = snoise(sampleCoords);
	sampleCoords.xyz -= elapsed * warper;

	for (int i = 0; i < NUM_STEPS; ++i)
	{
		float3 curPos = rayOrigin + t * rayDir;

		float dist = curPos.x * curPos.x + curPos.y * curPos.y + curPos.z * curPos.z;	//max 1

		float ratio1 = (1. - 3.f * dist);	//smoothstep(0.f, 2.0f, 1.f - dist);	//smoothstep(0.98f, 1.1f, dist)
		ratio1 = clamp(pow(ratio1, 5), 0., 1.);

		float ratio2 = smoothstep(0.95, 5.f, dist);

		density = turbulentFBM(sampleCoords, LACUNARITY, GAIN, NUM_OCTAVES);

		sum.ra += density * max(ratio1, ratio2);

		t += STEP_SIZE;

		//if (density >= .99f)	break;
	}

	return sum;
}


float4 main(PixelInputType input) : SV_TARGET
{
	float3 viewdir = normalize(input.wPos.xyz - eyePos.xyz);

	float4 colour raymarch(input.msPos.xyz, viewdir);

	return colour;
}