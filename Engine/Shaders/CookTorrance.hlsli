#include "Constants.hlsli"

// Made by courtesy of: 
// https://learnopengl.com/PBR/Theory
// http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx


float ndfGGXTR(float3 n, float3 h, float a)		//normal, halfway vector, roughness (const value/texture sample)
{
    float NdH = saturate(dot(n, h)); //check all these max operations... is this required?? can't tell trivially
    float NdH2 = NdH * NdH;
    float aa = a * a; //MIGHT WANT TO GO a^4 INSTEAD OF a^2, NOT SURE
    float toSq = NdH2 * (aa - 1.) + 1.;
	//float toSq = NdH * aa + (1. - NdH2);
    float denom = PI * toSq * toSq;
    return aa / denom;
}




//Can work for both metals and dielectricts but its hacky... I don't like it.
//f0 = ( (ni1 - ni2) / (ni1 + ni2) ) ^ 2 - should not be calculated in the shader though... just a reference
float3 fresnelSchlick(float cosTheta, float3 f0)
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
    return dfg / max(denom, EPS_def);
}


float3 diffuseLambert(float3 albedo)
{
    return albedo * PI_inv;
}


float3 brdfCookTorrance(float3 n, float3 v, float3 h, float3 l, float a, float3 albedo, float metallic, float3 f0)
{
    float3 ks = 0.f;
    float3 specular = specCookTorrance(n, v, h, l, a, f0, ks); //ks is basically fresnel
    float3 kd = float3(1., 1., 1.) - ks;
    kd *= (1. - metallic); //id prefer to separate metallic and dielectric PBR as two shaders tbh... but for now
    return kd * diffuseLambert(albedo) + specular;
}
