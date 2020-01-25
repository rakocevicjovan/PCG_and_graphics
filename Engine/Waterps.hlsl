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

float4 calcAmbient(in float3 alc, in float ali)
{
	return saturate(float4(alc, 1.0f) * ali);
}

float4 calcDiffuse(in float3 invLightDir, in float3 normal, in float3 dlc, in float dli, inout float dFactor)
{

	dFactor = max(dot(normal, invLightDir), 0.0f);
	return saturate(float4(dlc, 1.0f) * dli * dFactor);
}

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

	float4 texNormal = normalMap.Sample(Sampler, (input.texCoords + whirly));
	texNormal = 2.0f * texNormal - 1.f;
	input.tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
	float3 bitangent = cross(input.normal, input.tangent);
	float3x3 TBNMatrix = float3x3(input.tangent, bitangent, input.normal);
	input.normal = normalize(mul(texNormal.xyz, TBNMatrix));

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
	float4 specular = calcSpecular(-lightDir, input.normal, slc, sli, viewDir, dFactor);
	
	//float2 sampleCoords = NDC_xy + distortion;
	//sampleCoords = clamp(sampleCoords, 0.0001f, 0.9999f);

	float4 reflection = reflectionMap.Sample(Sampler, NDC_xy);		//clamp(NDC_xy + distortion, 0.000001, 0.999999)
	float4 refraction = refractionMappu.Sample(Sampler, NDC_xy);	//clamp(NDC_xy + distortion, 0.000001, 0.999999));

	float fresnel = dot(-viewDir, input.normal);	//refractive factor - low angle means it's lower, and pow decreases it further
	fresnel = fresnel * fresnel * fresnel * fresnel;

	float4 colour = lerp(reflection, refraction, fresnel);

	colour = (ambient * 20.f + diffuse) * colour + specular; //+ specular;


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
