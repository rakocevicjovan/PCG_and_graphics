cbuffer LightBuffer
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
};

Texture2D wnTexture : register(t0);
Texture2D reflectionMap : register(t1);
Texture2D refractionMap :register(t2);

SamplerState Sampler : register(s0);

float random(in float2 _st)
{
	return frac(sin(dot(_st.xy, float2(12.9898, 78.233))) * 43758.5453123);
}



// Based on Morgan McGuire @morgan3d
float noise2d(in float2 _st)
{

	float2 p = floor(_st);
	float2 f = frac(_st);

	//4 corners of a piece of 2d texture ran through the "random" function - it's actually deterministic tho
	float a = random(p);
	float b = random(p + float2(1.0, 0.0));
	float c = random(p + float2(0.0, 1.0));
	float d = random(p + float2(1.0, 1.0));

	float2 u = f * f * (3.0 - 2.0 * f);

	return lerp(a, b, u.x) + (c - a) * u.y * (1.0f - u.x) + (d - b) * u.x * u.y;
}



static const int NUM_OCTAVES = 5;

float fbm(in float2 hPos)
{
	float v = 0.0;
	float amplitude = 0.5;
	float2 shift = float2(100.0f, 100.0f);
	// Rotate to reduce axial bias
	float2x2 rot = float2x2(cos(0.5f), sin(0.5f), -sin(0.5f), cos(0.50f));
	for (int i = 0; i < NUM_OCTAVES; ++i) {
		v += amplitude * noise2d(hPos);
		hPos = mul(hPos, rot) * 2.0f + shift;
		amplitude *= 0.6f;
	}
	return v;
}



float4 strifeFragment(PixelInputType input) : SV_TARGET
{
	/*
	float3 eyeToFrag = input.worldPos.xyz - eyePos.xyz;
	float dist = length(eyeToFrag);
	eyeToFrag = eyeToFrag / dist;

	float3 lightDir = normalize(input.worldPos.xyz - lightPos);

	float normDist = dist / 1000.0f;

	//distort the colours
	float2 p = input.worldPos.xz * 0.033f;
	float2 q = float2(fbm(p + 0.3f * elapsed), fbm(p));
	float2 r = float2(fbm(p + q + float2(17, 92) + 0.66f * elapsed), fbm(p + 1.0f * q + float2(83, 28) + 0.66f * elapsed));
	float f = fbm(p + 2.f * q + 6.f * 1.0f * r);

	float3 colour = lerp(float3(0.15f, 0.3f, 0.45f), dli, clamp((f*f)*1.0f, 0.0f, 1.0f));	//lerp between diffuse light and dark water colours

	//apply scattering, but also darken a little if away from light for nicer contrast
	float scattering = clamp(dot(eyeToFrag, -lightDir), -0.1f, 1.0f);

	float uwotm8 = ( pow(f, 3.0f) + 0.66f * f * f + 0.33f * f);

	return float4((uwotm8 + 5 * scattering) * colour, uwotm8 * uwotm8 / exp(normDist * 5.0f));	//fades with distance relative to view frustum depth
	*/

	float4 colour = reflectionMap.Sample(Sampler, float2(input.texCoords.x, -input.texCoords.y));

	return colour;
}