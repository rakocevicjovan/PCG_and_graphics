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

Texture2D shaderTexture : register(t0);
SamplerState Sampler;

static const float SpecularPower = 8.f;


float4 main(PixelInputType input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float4 colour = shaderTexture.Sample(Sampler, input.tex);

	float3 invLightDir = normalize(lightPosition.xyz - input.worldPos.xyz);

	float3 viewDir = (input.worldPos.xyz - eyePos.xyz);

	float4 ambient = calcAmbient(alc, 1.);

	float dFactor = 0;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, dlc, 1., dFactor);

	float4 specular = calcSpecularPhong(invLightDir, input.normal, slc, 1., viewDir, dFactor, SpecularPower);

	colour = (ambient + diffuse) * colour + specular;

	colour.xyz = gammaCorrect(colour.xyz, 1.0f / 2.2f);
	colour.a = 1.f;

	return colour;
}