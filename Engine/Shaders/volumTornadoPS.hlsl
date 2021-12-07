#include "Noise.hlsli"
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


//Constants
static const float TWISTER = 12.;
//FBM settings
static const int NUM_OCTAVES = 3;
static const float LACUNARITY = 1.13795;
static const float GAIN = .797531;


//Raymarch settings
static const int NUM_STEPS = 33;
static const float STEP_SIZE = sqrt(2) / (float) NUM_STEPS;



float4 raymarch(in float3 rayOrigin, in float3 rayDir, in float2x2 rotMat)
{
    float4 sum = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float particle;

    float flame = 0.f;
    float t = 0.f;
    float3 noiseDir = float3(0., .66, .33);

    for (int i = 0; i < NUM_STEPS; ++i)
    {
        if (flame > .99f)
            break;

        float3 curPos = rayOrigin + t * rayDir;

        curPos.xz *= 4.f;
        float mask = clamp(-sdCylinder(curPos, float3(0., 0., 1.)), 0., 1.);

        mask *= turbulentFBM(curPos * 2.f - float3(0, elapsed, 0) * 3.f, LACUNARITY, GAIN, NUM_OCTAVES);

        mask = pow(mask, .5);

        flame += STEP_SIZE * mask;

        t += STEP_SIZE;
    }

    return (float4) flame;

	//sum = smoothstep(float4(0., 0., 0., 0.), float4(1.2, 1.2, 1.2, 1.), float4(.2 * flame , flame, .7f * flame, flame));
    float g = smoothstep(.3, 1.6, flame);
    float rb = g * g;
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