#include "Simplex.hlsli"
#include "Noise.hlsli"


cbuffer VariableBuffer : register(b0)
{
	float elapsed;
	float3 padding;
};


cbuffer ViewDirBuffer : register(b1)
{
	float4x4 rotationMatrix;
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

//Helpers
static const float PI = 3.141592f;
static const float INTENSITY = 1.61803f * PI;
static const float3 PARTICLE_OFFSET = float3(-.5, .707, -.33);

//FBM settings
static const int NUM_OCTAVES = 7;
static const float LACUNARITY = 2.13795;
static const float GAIN = .497531;

//Raymarch settings
static const int NUM_STEPS = 7;
static const float STEP_SIZE = 2.f / (float)NUM_STEPS;

float4 raymarch(in float3 rayOrigin, in float3 rayDir)
{
	float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float particle;

	float flame = 0.f;
	float t = 0.f;
	float3 noiseDir = float3(0., .66, .33);
	
	for (int i = 0; i < NUM_STEPS; ++i)
	{
		if (flame > 95.f)	break;

		float3 curPos = rayOrigin + t * rayDir;

		//float3 curPosAdj = float3(abs(curPos.x), curPos.y, curPos.z);
		float3 rotatedPos = mul(curPos, rotationMatrix);
		float3 noisePos = rotatedPos - noiseDir * elapsed;
		
		float dist = length(curPos) - .33f * snoise(noisePos);	//get dist to middle of sphere
		float mask = smoothstep(.707, 0., dist);
		
		flame += 
				STEP_SIZE * 
				mask * 
				turbulentFBM(noisePos * 2.1f, LACUNARITY, GAIN, NUM_OCTAVES) *
				INTENSITY;

		t += STEP_SIZE;
	}

	sum = smoothstep(float4(0., 0., 0., 0.), float4(1.2, 1.2, 1.2, 1.), float4(flame, flame * flame, .7f * flame * flame, flame));
	
	return sum;
}



float4 main(PixelInputType input) : SV_TARGET
{
	float3 viewdir = normalize(input.wPos.xyz - eyePos.xyz); //ray direction

	float4 colour = raymarch(input.msPos.xyz, viewdir);	//ray origin is the first pixel hit

	return colour;
}