#include "Constants.hlsli"
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
	float4 stupidThingIUsedToDo;
};

cbuffer PSPerFrameBuffer : register(b10)
{
	float4 eyePos;
	float elapsed;
	float delta;
	float2 padding;
}


struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
	//float3 tangent : TANGENT;
};

/* 
// either pack data as a constant for the whole mesh or use textures
cbuffer PBRBuffer : register(b1)
{
	float metallic;
	float3 materialColour;
}
*/

/* bootleg models and materials for now, need to improvise, using textures as below to test the shader
Texture2D diffuseMap : register(t0);
Texture2D surfaceMap : register(t1);
Texture2D normalMap : register(t2);
*/

Texture2D<float4> diffuseMap : register(t0);
Texture2D<float4> metalMap : register(t1);
Texture2D<float4> roughMap : register(t2);
Texture2D<float4> ambOcMap : register(t3);


SamplerState Sampler : register(s0);


// Made by courtesy of : 
// https://learnopengl.com/PBR/Theory
// http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx


float ndfGGXTR(float3 n, float3 h, float a)		//normal, halfway vector, roughness (const value/texture sample)
{
	float NdH = saturate(dot(n, h));		//check all these max operations... is this required?? can't tell trivially
	float NdH2 = NdH * NdH;
	float aa = a * a;							//MIGHT WANT TO GO a^4 INSTEAD OF a^2, NOT SURE
	float toSq = NdH2 * (aa - 1.) + 1.;
	//float toSq = NdH * aa + (1. - NdH2);
	float denom = PI * toSq * toSq;
	return aa / denom;
}




//Can work for both metals and dielectricts but its hacky... I don't like it.
//F0 = ( (ni1 - ni2) / (ni1 + ni2) ) ^ 2 - should not be calculated in the shader though... just a reference
float3 fresnelSchlick(float cosTheta, float3 f0)	//can pass float cosTheta or float3 h, float3 v,...
{
	cosTheta = min(cosTheta, 1.0);
	return max(f0 + (float3(1., 1., 1.) - f0) * pow(1. - cosTheta, 5.), 0.);
}


float kDirect(float a)
{
	float ap1 = a + 1.;
	return (ap1 * ap1) / 8.;
}


float kIBL(float a)
{
	return a * a / 2.;
}


float geometrySchlickGGX(float NdX, float a)	//substitute X once with V, once with L and combine in geometrySmith
{
	float k = kDirect(a);
	float denom = NdX * (1. - k) + k;
	return NdX / denom;
}


float geometrySmith(float3 n, float3 v, float3 l, float a)
{
	float NdV = saturate(dot(n, v));
	float NdL = saturate(dot(n, l));
	float ggx1 = geometrySchlickGGX(NdV, a);
	float ggx2 = geometrySchlickGGX(NdL, a);
	return ggx1 * ggx2;
}


float3 specCookTorrance(float3 n, float3 v, float3 h, float3 l, float a, in float3 f0, inout float3 fresnel)
{
	fresnel = fresnelSchlick(max(dot(h, v), 0.), f0);
	float3 dfg = ndfGGXTR(n, h, a) * fresnel * geometrySmith(n, v, l, a);
	float denom = 4 * saturate(dot(n, v)) * saturate(dot(n, l));
	//denom = 1.;
	return dfg / max(denom, EPS_def);
}


float3 diffuseLambert(float3 albedo)
{
	return albedo * PI_inv;
}


float3 brdfCookTorrance(float3 n, float3 v, float3 h, float3 l, float a, float3 albedo, float metallic, float3 f0)
{
	float3 ks = 0.f;
	float3 specular = specCookTorrance(n, v, h, l, a, f0, ks);	//ks is basically fresnel
	float3 kd = float3(1., 1., 1.) - ks;
	kd *= (1. - metallic);		//id prefer to separate metallic and dielectric PBR as two shaders tbh... but for now
	return kd * diffuseLambert(albedo) + specular;
}


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


	//aww yeah it's all coming together

	// tutorial says to multiply by N dot L but really?? Seems like it's already been accounted for to me...
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