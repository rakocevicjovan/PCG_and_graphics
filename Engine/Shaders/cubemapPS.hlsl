#include "Light.hlsli"


struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
};

TextureCube cubeMapTexture;
SamplerState Sampler;

static const float SpecularPower = 64.f;

float4 main(PixelInputType input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);
	float3 invLightDir = -lightDir;
	float3 viewDir = normalize(input.worldPos.xyz - eyePos.xyz);	

	float3 incident = viewDir;
	float3 reflected = reflect(incident, input.normal);

	float4 colour = cubeMapTexture.Sample(Sampler, reflected);

	float4 ambient = calcAmbient(alc, ali);
	float dFactor = 0.f;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, dlc, dli, dFactor);
	float4 specular = calcSpecularPhong(invLightDir, input.normal, slc, sli, viewDir, dFactor, SpecularPower);
	colour = (ambient + diffuse) * colour + specular;

	colour.rgb = pow( colour.xyz, float3(0.333, 0.333, 0.333));

    return colour;
}