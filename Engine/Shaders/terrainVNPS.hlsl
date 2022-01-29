#include "Light.hlsli"
#include "MathUtils.hlsli"

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
	float3 tangent : TANGENT;
};

Texture2D tex0 : register(t0);
Texture2D normalMap : register(t1);

SamplerState Sampler : register(s0);

//go 2-4 times higher on this when using blinn phong compared to phong
static const float SpecularPower = 32.f;

float4 main(PixelInputType input) : SV_TARGET
{
	mapNormals(Sampler, normalMap, input.tex, input.tangent, input.normal);
	
    float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);
	float3 invLightDir = -lightDir;

	float3 viewDir = input.worldPos.xyz - eyePos.xyz;
	float distance = length(viewDir);
	viewDir = viewDir / distance;
	float3 invViewDir = -viewDir;

	float4 colour = tex0.Sample(Sampler, input.tex);

	//calculate ambient light
	float4 ambient = calcAmbient(alc, ali);

	//calculate diffuse light
	float dFactor = 0.f;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, dlc, dli, dFactor);

	//calculate specular light
	float4 specular = calcSpecularPhong(invLightDir, input.normal, slc, sli, viewDir, dFactor, SpecularPower);

	colour = (ambient + diffuse) * colour + specular;

	colour.xyz = gammaCorrect(colour.xyz, 1.0f / 2.2f);
	colour.w = 1.f;

	return colour;
}