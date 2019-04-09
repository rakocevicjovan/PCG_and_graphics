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
	float4 clipSpace : CLIPPY;
	float3 tangent : TANGENT;
};

Texture2D normalMap : register(t0);
Texture2D reflectionMap : register(t1);
Texture2D refractionMappu :register(t2);
SamplerState Sampler : register(s0);

static const float PI = 3.141592f;
static const float INTENSITY = 1.61803f * PI;
static const float TWISTER = 5.;
static const int NUM_OCTAVES = 5;

float4 calcAmbient(in float3 alc, in float ali)
{
	return saturate(float4(alc, 1.0f) * ali);
}


float4 calcDiffuse(in float3 invLightDir, in float3 normal, in float3 dlc, in float dli, inout float dFactor)
{

	dFactor = max(dot(normal, invLightDir), 0.0f);
	return saturate(float4(dlc, 1.0f) * dli * dFactor);
}

static const float distortionIntensity = .02f;
static const float SpecularPower = 8.f;

float4 calcSpecular(in float3 invLightDir, in float3 normal, in float3 slc, in float sli, in float3 invViewDir, in float dFactor)
{
	float3 reflection = normalize(reflect(invLightDir, normal));
	float sFactor = pow(saturate(dot(reflection, invViewDir)), SpecularPower);
	return saturate(float4(slc, 1.0f) * sFactor * sli * dFactor);	// sli
}

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

	//4 corners of a piece of 2d texture ran through the "random" function - it's actually deterministic tho
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
	float amplitude = 0.5;
	float2 shift = float2(100.0f, 100.0f);
	// Rotate to reduce axial bias
	for (int i = 0; i < NUM_OCTAVES; ++i) {
		v += amplitude * noise2d(hPos);
		hPos = mul(hPos, rotMat) * 2.0f + shift;
		amplitude *= 0.6f;
	}
	return v;
}


float4 strifeFragment(PixelInputType input) : SV_TARGET
{	
	float2 p = input.worldPos.xz * 0.066f;
	float2 q = float2(fbm(p + 0.2f * elapsed), fbm(p));
	float2 r = float2(fbm(p + q + float2(17, 92) + 0.66f * elapsed), fbm(p + 1.0f * q + float2(83, 28) + 0.66f * elapsed));
	float whirly = fbm(p + 2.f * q + 6.f * 1.0f * r);


	float4 texNormal = normalMap.Sample(Sampler, (input.texCoords + whirly));
	texNormal = 2.0f * texNormal - 1.f;
	input.tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
	float3 bitangent = cross(input.normal, input.tangent);
	float3x3 TBNMatrix = float3x3(input.tangent, bitangent, input.normal);
	input.normal = normalize(mul(texNormal, TBNMatrix));
	
	//ndc being [-1, 1] range, then remapped into texture coordinate range

	float2 distortion = pow(input.normal.xy, 1) * distortionIntensity;

	float2 NDC_xy;
	NDC_xy.x = input.clipSpace.x / input.clipSpace.w;
	NDC_xy.y = -input.clipSpace.y / input.clipSpace.w;
	NDC_xy /= float2(2.0f, 2.0f);
	NDC_xy += float2(0.5f, 0.5f);
	NDC_xy += distortion;
	

	//light
	float3 viewDir = normalize(input.worldPos.xyz - eyePos.xyz);

	float3 lightDir = normalize(input.worldPos.xyz - lightPos.xyz);
	float4 ambient = calcAmbient(alc, ali);
	float dFactor = 0.f;
	float4 diffuse = calcDiffuse(-lightDir, input.normal, dlc, dli, dFactor);

	//calculate specular light
	float4 specular = calcSpecular(-lightDir, input.normal, slc, sli, viewDir, dFactor);

	float4 reflection = reflectionMap.Sample(Sampler, NDC_xy);
	float4 refraction = refractionMappu.Sample(Sampler, NDC_xy);
	float4 colour = lerp(reflection, refraction, 0.5);

	colour = colour + specular; //+ specular;

	return colour;
}




//static const float si = sin(elapsed * .61f);
//static const float co = cos(elapsed * .61f);
//static const float2x2 rotMat = float2x2(co, si, -si, co);

/*	

	float3 colour = lerp(float3(0.15f, 0.3f, 0.45f), dli, clamp((f*f)*1.0f, 0.0f, 1.0f));	//lerp between diffuse light and dark water colours
	apply scattering, but also darken a little if away from light for nicer contrast
	float scattering = clamp(dot(viewDir, -lightDir), -0.1f, 1.0f);
	float uwotm8 = ( pow(f, 3.0f) + 0.66f * f * f + 0.33f * f);
	return float4((uwotm8 + 5 * scattering) * colour, uwotm8 * uwotm8 / exp(normDist * 5.0f));	//fades with distance relative to view frustum depth
	*/


/*

// Based on Morgan McGuire @morgan3d

float fbm(in float2 hPos)
{
	float v = 0.0;
	float amplitude = 0.5;
	float2 shift = float2(100.0f, 100.0f);
	// Rotate to reduce axial bias
	for (int i = 0; i < NUM_OCTAVES; ++i) {
		v += amplitude * noise2d(hPos);
		hPos = mul(hPos, rotMat) * 2.0f + shift;
		amplitude *= 0.6f;
	}
	return v;
}

	float2 p = input.worldPos.xz * 0.033f;
	float2 q = float2(fbm(p + 0.3f * elapsed), fbm(p));
	float2 r = float2(fbm(p + q + float2(17, 92) + 0.66f * elapsed), fbm(p + 1.0f * q + float2(83, 28) + 0.66f * elapsed));
	float f = fbm(p + 2.f * q + 6.f * 1.0f * r);
*/
