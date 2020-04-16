#include "Light.hlsli"

Texture2D depthMapTexture : register(t0);
Texture2D shaderTexture : register(t1);


SamplerState SampleTypeWrap  : register(s0);
SamplerState SampleTypeClamp : register(s1);


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
	float4 worldPosition : TEXTURE;
    float4 fragPosLightSpace : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};

static const float SpecularPower = 8.f;


float3 darken(in float3 rgb)
{
	return float3(lerp(rgb, float3(0.0f, 0.0f, 0.0f), 0.5f));
}


float4 main(PixelInputType input) : SV_TARGET
{
    float2 projectTexCoord;
    float closestDepth;
    float lightDepthValue;
    float lightIntensity;

	input.normal = normalize(input.normal);

	float4 texColour = shaderTexture.Sample(SampleTypeWrap, input.tex);
	float4 colour;

	//fragPosLightSpace is perspective divided and transformed from [-1, 1] to [0, 1] range in light space
    projectTexCoord.x =  input.fragPosLightSpace.x / input.fragPosLightSpace.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.fragPosLightSpace.y / input.fragPosLightSpace.w / 2.0f + 0.5f;

	float3 lightDir = normalize(input.worldPosition.xyz - lightPosition.xyz);
	float3 invLightDir = -lightDir;

	float3 viewDir = input.worldPosition.xyz - eyePos.xyz;
	float distance = length(viewDir);
	viewDir = viewDir / distance;
	float3 invViewDir = -viewDir;

	//calculate ambient light
	float4 ambient = calcAmbient(alc, ali);

	//calculate diffuse light
	float dFactor = 0;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, dlc, dli, dFactor);

	//calculate specular light
	float4 specular = calcSpecularPhong(invLightDir, input.normal, slc, sli, viewDir, dFactor, SpecularPower);

	colour = (ambient + diffuse) * texColour;

	//apply fog
	colour.xyz = applyFog(colour.xyz, distance, viewDir, lightDir);

	//apply gamma correction
	colour.xyz = gammaCorrect(colour.xyz, 1.0f / 2.2f);


    if((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y)){

        closestDepth = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;
        //lightDepthValue = ( (2.f * input.fragPosLightSpace.z - 500.01f ) / 499.99f ) / input.fragPosLightSpace.w;
		lightDepthValue = input.fragPosLightSpace.z / input.fragPosLightSpace.w;
        lightDepthValue -= 0.001f;

		if (lightDepthValue > closestDepth)
		{
			colour.xyz = darken(colour.xyz);
			colour.w = 1.0f;
			return colour;
		}
	}


	colour = colour + specular;
	colour.w = 1.0f;
	return colour;
}
