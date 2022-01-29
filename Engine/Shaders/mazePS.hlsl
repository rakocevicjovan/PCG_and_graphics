#include "Light.hlsli"

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
	float3 tangent : TANGENT;
};

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
SamplerState Sampler;



float4 main(PixelInputType input) : SV_TARGET
{
	float4 colour = diffuseMap.Sample(Sampler, input.tex);

	mapNormals(Sampler, normalMap, input.tex, input.tangent, input.normal);

	//use the normal in regular light calculations now
	float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);
	float3 invLightDir = -lightDir;

	float3 viewDir = input.worldPos.xyz - eyePos.xyz;
	float distance = length(viewDir);
	viewDir = viewDir / distance;
	float3 invViewDir = -viewDir;

	//calculate ambient light
	float4 ambient = calcAmbient(alc, ali);

	//calculate diffuse light
	float dFactor = 0.f;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, .1f, dli, dFactor);

	//calculate specular light
	float4 specular = calcSpecularPhong(invLightDir, input.normal, .3f, sli, viewDir, dFactor, SpecularPower);

	colour = (ambient + diffuse) * colour + specular;

	//apply gamma correction
	colour.rgb = gammaCorrect(colour.rgb, 1.0f / 3.2f);

	colour.a = 1.f;
	return colour;
}