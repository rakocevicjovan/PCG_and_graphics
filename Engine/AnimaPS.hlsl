#include "Light.hlsli"

cbuffer LightBuffer
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

Texture2D shaderTexture;
SamplerState SampleType;


static const float SpecularPower = 8.f;


float4 main(PixelInputType input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);
	float3 invLightDir = -lightDir;

	float3 viewDir = input.worldPos.xyz - eyePos.xyz;
	float distance = length(viewDir);
	viewDir = viewDir / distance;
	float3 invViewDir = -viewDir;

	//texture colour
	float4 colour = shaderTexture.Sample(SampleType, input.tex);

	//calculate ambient light
	float4 ambient = calcAmbient(alc, ali);

	//calculate diffuse light
	float dFactor = 0;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, dlc, dli, dFactor);

	//calculate specular light
	float4 specular = calcSpecularPhong(invLightDir, input.normal, slc, sli, viewDir, dFactor, SpecularPower);

	colour = (ambient + diffuse) * colour + specular;

	//apply fog
	colour = float4(applyFog(colour.xyz, distance, viewDir, lightDir), 1.0f);

	//apply gamma correction
	colour.xyz = gammaCorrect(colour.xyz, 1.0f / 2.2f);

	return colour;
}