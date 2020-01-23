#include "MathUtils.hlsli"
#include "Simplex.hlsli"
#include "SDF.hlsli"

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


//Helpers
static const float PI = 3.141592f;
static const float INTENSITY = 1.61803f * PI;
static const float TWISTER = 5.;

//FBM settings
static const int NUM_OCTAVES = 3;
static const float LACUNARITY = 1.13795;
static const float GAIN = .797531;

float turbulentFBM(float3 x)
{
	float sum = 0.0f;

	float frequency = 1.0f;
	float amplitude = 1.0f;

	for (int i = 0; i < NUM_OCTAVES; ++i)
	{
		float r = snoise(frequency * x) * amplitude;
		r = r < 0 ? -r : r;
		sum += r;
		frequency *= LACUNARITY;
		amplitude *= GAIN;
	}

	return sum;
}

//here for reference, changed it slightly
float3 opTwist(in float3 p)
{
	float nani = TWISTER * p.y - elapsed * .66;
	float c = cos(nani);
	float s = sin(nani);
	float2x2 rotoMato = float2x2(c, -s, s, c);
	return  float3(mul(p.xz, rotoMato), p.y);
}


//Raymarch settings
static const int NUM_STEPS = 33;
static const float STEP_SIZE = 2.f / (float)NUM_STEPS;


float4 raymarch(in float3 rayOrigin, in float3 rayDir, in float2x2 rotMat)
{
	float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float particle;

	float flame = 0.f;
	float t = 0.f;
	float3 noiseDir = float3(0., .66, .33);

	for (int i = 0; i < NUM_STEPS; ++i)
	{
		if (flame > .99f)	break;

		float3 curPos = rayOrigin + t * rayDir;

		float3 twisted = opTwist(curPos);

		float mask = 1.f - sdTorus(twisted, float2(0.4, 0.05));
		mask *= turbulentFBM(twisted - elapsed);

		mask = pow(mask, 5);

		flame += STEP_SIZE * mask;

		t += STEP_SIZE;
	}

	//sum = smoothstep(float4(0., 0., 0., 0.), float4(1.2, 1.2, 1.2, 1.), float4(.2 * flame , flame, .7f * flame, flame));
    float g = smoothstep(.0, .7, flame);
    float rb = min(flame * flame, g);
    sum = float4(rb, g, rb, g);

	return sum;
}


float4 main(PixelInputType input) : SV_TARGET
{
	float3 xyz = input.msPos.xyz;

	float3 viewdir = normalize(input.wPos.xyz - eyePos.xyz); //ray direction

	float nani = TWISTER * xyz.y + elapsed;
	float c = cos(nani);
	float s = sin(nani);
	float2x2 rotMat = float2x2(c, -s, s, c);

	float4 colour = raymarch(xyz, viewdir, rotMat);

	return colour;
}