#include "Light.hlsli"

cbuffer LightBuffer : register(b0)
{
	float3 alc;
	float ali;
	float3 dlc;
	float dli;
	float3 slc;
	float sli;
	float4 lightPos;
	float4 eyePos;
	float elapsed;
	float3 padding;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
	float4 clipSpace : CLIPSPACE;
	float3 tangent : TANGENT;
};

Texture2D normalMap : register(t0);
Texture2D reflectionMap : register(t1);
Texture2D refractionMappu :register(t2);
SamplerState Sampler : register(s0);

static const int NUM_OCTAVES = 5;
static const float SpecularPower = 8.f;
static const float DISTORTION_INTENSITY = .033333f;


static const float co = cos(.5f);
static const float si = sin(.5f);
static const float2x2 rotMat = float2x2(co, si, -si, co);

float random(in float2 _st)
{
	return frac(sin(dot(_st.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

float noise2d(in float2 _st)
{
	float2 p = floor(_st);
	float2 f = frac(_st);

	//4 corners of a piece of 2d texture ran through the pseudo-random function function
	float a = random(p);
	float b = random(p + float2(1.0, 0.0));
	float c = random(p + float2(0.0, 1.0));
	float d = random(p + float2(1.0, 1.0));

	float2 u = f * f * (3.0 - 2.0 * f);

	return lerp(a, b, u.x) + (c - a) * u.y * (1.0f - u.x) + (d - b) * u.x * u.y;
}

float fbm(in float2 hPos)
{
	float v = 0.0;
	float amplitude = 1.0;
	float frequency = 1.f;
	float gain = .5973f;
	float lacunarity = 1.01f;
	float2 shift = float2(100.0f, 100.0f);
	// Rotate to reduce axial bias
	for (int i = 0; i < NUM_OCTAVES; ++i)
	{
		v += amplitude * noise2d(frequency * hPos);
		hPos = mul(hPos, rotMat) * 2.0f + shift;
		amplitude *= gain;
		frequency *= lacunarity;
	}
	return v;
}


float4 main(PixelInputType input) : SV_TARGET
{	
	float time = elapsed * .2f;

	float2 p = input.worldPos.xz * 0.03f;
	float2 q = float2(fbm(p + time), fbm(p));
	float2 r = float2(fbm(p + q + time), fbm(p + 1.0f * q + time));
	float whirly = fbm(p + 2.f * q + 6.f * 1.0f * r);

	mapNormals(Sampler, normalMap, input.texCoords + whirly, input.tangent, input.normal);

	float2 distortion = ((input.normal.xy * 2.0f) - 1.0f) * DISTORTION_INTENSITY;

	float2 NDC_xy;
	NDC_xy.x =  input.clipSpace.x / input.clipSpace.w / 2.f + 0.5f;
	NDC_xy.y = -input.clipSpace.y / input.clipSpace.w / 2.f + 0.5f;
	
	//light
	float3 viewDir = normalize(input.worldPos.xyz - eyePos.xyz);
	float3 lightDir = normalize(input.worldPos.xyz - lightPos.xyz);
	float4 ambient = calcAmbient(alc, ali);
	float dFactor = 0.f;
	float4 diffuse = calcDiffuse(-lightDir, input.normal, dlc, dli, dFactor);
	float4 specular = calcSpecularPhong(-lightDir, input.normal, slc, sli, viewDir, dFactor, SpecularPower);
	
	float4 reflection = reflectionMap.Sample(Sampler, NDC_xy);
	float4 refraction = refractionMappu.Sample(Sampler, NDC_xy);

	float fresnel = dot(-viewDir, input.normal);	//refractive factor - low angle means it's lower, and pow decreases it further
	fresnel = pow(fresnel, 4);

	float4 colour = lerp(reflection, refraction, fresnel);

	colour = (ambient * 20.f + diffuse) * colour + specular;

	return colour;
}