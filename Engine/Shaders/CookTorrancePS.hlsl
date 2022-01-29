#include "Reserved_CB_PS.hlsli"
#include "CookTorrance.hlsli"
#include "Light.hlsli"


// This shader is an absolute mess by now
// Strip out useful parts into helper function file PBR.hlsli

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
	//float3 tangent : TANGENT;
};

/* 
	// OPTION 1 - pack data as a constant for the whole mesh
	cbuffer PBRBuffer : register(b1)
	{
		float metallic;
		float3 materialColour;
	}

	// OPTION 2 - use textures
	Bootleg models and materials for now, need to improvise, using textures below to test the shader
	Texture2D diffuseMap : register(t0);
	Texture2D surfaceMap : register(t1);
	Texture2D normalMap : register(t2);
*/

Texture2D<float4> diffuseMap : register(t0);
Texture2D<float4> metalMap : register(t1);
Texture2D<float4> roughMap : register(t2);
Texture2D<float4> ambOcMap : register(t3);

SamplerState Sampler : register(s0);


float4 main(PixelInputType input) : SV_TARGET
{
	// Per pixel/fragment properties
	float3 n = normalize(input.normal);
	float3 toEye = normalize(eyePos.xyz - input.worldPos.xyz);

	// Per material sample properties

	// texture containing ONLY colour (no prebaked lighting)
	float3 albedo = diffuseMap.Sample(Sampler, input.tex).xyz;

	//albedo to linear space
	//albedo = pow(albedo, float3(2.2, 2.2, 2.2));

	// for float3 packed texture with refraction index delta at r, roughness at g and metallic property on b
	/*
	float3 surface = surfaceMap.Sample(Sampler, input.tex).xyz;
	float refIndex = 1.f + surface.r;
	float roughness = saturate(surface.g - EPS_def) + EPS_def;
	float metallic = surface.b;
	*/

	float refIndex = 1.05f;	//assuming metal for now dont have textures...
	float metallic = metalMap.Sample(Sampler, input.tex).x;
	float roughness = roughMap.Sample(Sampler, input.tex).x;
	roughness = saturate(roughness - EPS_def) + EPS_def;

	// calculate F0, should make a permutation with constant materialColour rgb and with a texture
	//float3 f0 = abs((1. - refIndex) / (1. + refIndex));	//0.04 is usually good enough for dielectrics, metals need a this specified
	//f0 = f0 * f0;
	
	#define F_F0 0.04f
	float3 f0 = float3(F_F0, F_F0, F_F0);
	f0 = lerp(f0, albedo, metallic);	//metals have tinted reflections

	// Per light based properties
	float3 toLight = lightPosition.xyz - input.worldPos.xyz;
	float lightDist = length(toLight);
	toLight = toLight / lightDist;

	float3 h = normalize(toEye + toLight);

	float attenuation = 1.f / (lightDist * lightDist);		//square falloff
	float3 radiance = slc * 20000.f * attenuation;

	// Aww yeah it's all coming together

	// Tutorial says to multiply by N dot L but really?? Seems like it's already been accounted for to me...
	float3 colour = brdfCookTorrance(n, toEye, h, toLight, roughness, albedo, metallic, f0) * radiance * saturate(dot(n, toLight));

	colour = colour / (colour + float3(1., 1., 1.));
	colour = gammaCorrect(colour, 1. / 2.2);
	return float4(colour, 1.);
}


/*
//ok this is suspicious because none of these values can be rgb... and one ought to if I understand the theory correctly!
//assumes that light is coming from air/vaccum environment, not submerged or anything...
float fresnelMetal(float ni, float k, float cosTheta)	//k is the absorbtion coefficient of the metal
{
	return ((ni - 1.) * (ni - 1.) + 4. * ni * pow(1. - cosTheta, 5) + k * k) / ((ni + 1.) * (ni + 1.) + k * k);
}
*/