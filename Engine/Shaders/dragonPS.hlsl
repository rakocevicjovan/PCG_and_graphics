#include "Simplex.hlsli"
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
    float4 eyePos;
};


struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 worldPos : WPOS;
};

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
SamplerState Sampler;


#define OCTAVES 8
#define GAIN .79
#define LACUNARITY 3.14159

float turbulentFBM(float3 x)
{
    float sum = 0.0f;

    float frequency = 2.0f;
    float amplitude = 1.0f;

    for (int i = 0; i < OCTAVES; ++i)
    {
        float r = snoise(frequency * x) * amplitude;
        sum += r;
        frequency *= LACUNARITY;
        amplitude *= GAIN;
    }

    return sum;
}


float4 main(PixelInputType input) : SV_TARGET
{
    input.normal = normalize(input.normal);

    float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);

    float3 viewDir = input.worldPos.xyz - eyePos.xyz;
    float distance = length(viewDir);
    viewDir = viewDir / distance;
    float3 invViewDir = -viewDir;

    float4 colour = tex0.Sample(Sampler, input.tex);
    colour += turbulentFBM(input.worldPos.xyz / 10.f);

    float4 ambient = calcAmbient(alc, ali);
    float dFactor = 0;
    float4 diffuse = calcDiffuse(-lightDir, input.normal, dlc, dli, dFactor);
    float4 specular = calcSpecular(-lightDir, input.normal, slc, sli, viewDir, dFactor, 8.f);
    colour.xyz = (ambient.xyz + diffuse.xyz) * colour.xyz + specular.xyz;
    colour.xyz = gammaCorrect(colour.xyz, 1.f / 2.2f);
 
    return colour;
}


/*
#define STEPS 12
#define STEP_SIZE 1.f / (float)STEPS

float4 colour = (float4) 0.0f;

float r = (1.f - dot(input.normal, invLightDir)) * STEP_SIZE;

for (int i = 0; i < STEPS; ++i)
{
    float3 curPos = input.worldPos.xyz + i * STEP_SIZE * viewDir;

    float g = turbulentFBM(curPos / 33.f) * STEP_SIZE + r * .2f;
    float b = .6f - max(r, g);
    colour.rg += float2(r, g);
    colour.b += b * STEP_SIZE;
    colour.a += max(g, b);
}*/