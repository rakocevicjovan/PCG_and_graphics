#include "Constants.hlsli"

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
	float3 tangent : TANGENT;
};

Texture2D diffuseMap : register(t0);
Texture2D metallicRoughness : register(t1);
Texture2D normalMap : register(t2);


// Made by courtesy of : 
// https://learnopengl.com/PBR/Theory
// http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx


float ndfGGXTR(float3 n, float3 h, float a)	//normal, halfway vector, roughness (const value/texture sample)
{
	float nh = max(dot(n, h), 0.f);
	float toSq = (nh * nh) * (a * a - 1.) + 1.;
	float denom = PI * toSq * toSq;
	return (a * a) / denom;
}




//only works for dielectrics, not conductors! For metals, the base reflectivity (F0) is wavelength dependent (rgb value)
//F0 = ( (ni1 - ni2) / (ni1 + ni2) ) ^ 2 - should not be calculated in the shader though... just a reference
float fresnelSchlick(float3 h, float3 v, float F0)	//can also just pass cosTheta...
{
	return F0 + (1. - F0) * pow(1. - dot(h, v), 5);
}


//ok this is suspicious because im not sure which value needs to be rgb... aka how it pertains to the above comment and function
//assumes that light is coming from air/vaccum environment, not submerged or anything... 
float fresnelMetal(float ni, float k, float cosTheta)	//k is the absorbtion coefficient of the metal
{
	return ((ni - 1.) * (ni - 1.) + 4. * ni * pow(1. - cosTheta, 5) + k * k) / ((ni + 1.) * (ni + 1.) + k * k);
}


float kDirect(float a)
{
	float ap1 = a + 1;
	return ap1 * ap1 / 8.;
}


float kIBL(float a)
{
	return a * a / 2.;
}


float geometrySchlickGGX(float nDotV, float k)
{
	float nom = nDotV;
	float denom = nom * (1. - k) + k;

	return nom / denom;
}


float geometrySmith(float3 n, float3 v, float3 l, float k)
{
	float nDotV = max(dot(n, v), 0.);
	float nDotL = max(dot(n, l), 0.);
	float ggx1 = geometrySchlickGGX(nDotV, k);
	float ggx2 = geometrySchlickGGX(nDotL, k);
	return ggx1 * ggx2;
}


float3 specCookTorrance(float3 n, float3 v, float k, float3 h, float a, inout float fresnel)
{
	fresnel = fresnelSchlick(h, v, F0);
	return (ndfGGXTR(n, h, a) * fresnel * geometrySmith(n, v, l, k) / (4 * (w0 * n) * (wI * n));
}


float3 diffuseLambert(float3 c)
{
	return c * PI_inv;
}


float3 brdfCookTorrance(float3 n, float3 h, float a)
{
	float ks = 0.f;
	float3 specular = specCookTorrance(n, v, k, h, a, ks);	//ks is calculated as fresnel
	float kd = 1. - ks;
	return kd * diffuseLambert() + specular;		//fLambert = c / PI
}


float4 main(PixelInputType input) : SV_TARGET
{
	//assumes float3 packed texture with refraction index delta at r, roughness at g and metallic property on b
	float3 surface = diffuseMap.Sample(Sampler, input.tex);
	float refIndex = 1.f + surface.r;
	float roughness = saturate(surface.g - EPS_def) + EPS_def;
	float metallic = surface.b;

	//calculate F0, should make a permutation with constant materialColour rgb and with a texture
	float3 F0 = abs((1. - refIndex) / (1. + refIndex));
	F0 = F0 * F0;
	F0 = lerp(F0, materialColour.rgb, metallic);


	brdfCookTorrance(n, h, a);


	float4 colour = { 1., 1., 1., 1. };

	//remember to do gamma correction
	return colour;
}