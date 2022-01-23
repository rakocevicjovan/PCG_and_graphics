//a bunch of utility function that deal with light calculations


float4 calcAmbient(in float3 alc, in float ali)
{
	return saturate(float4(alc, 1.0f) * ali);
}


float4 calcDiffuse(in float3 invLightDir, in float3 normal, in float3 dlc, in float dli, inout float diffIntensity)
{
	diffIntensity = max(dot(normal, invLightDir), 0.0f);
	return saturate(float4(dlc, 1.0f) * dli * diffIntensity);
}


float4 calcSpecularPhong(in float3 invLightDir, in float3 normal, in float3 slc, in float sli, in float3 invViewDir, in float diffIntensity, in float sp)
{
	float3 reflection = normalize(reflect(invLightDir, normal));
	float specIntensity = pow(saturate(dot(reflection, invViewDir)), sp);
	return saturate(float4(slc, 1.0f) * specIntensity * sli * diffIntensity);
}




float3 gammaCorrect(in float3 colour, in float3 gammaFactor)
{
	return pow(colour.xyz, gammaFactor);
}


float3 gammaCorrect(in float3 colour, in float gammaFactor)	//convenient overload nothing more
{
	return pow(colour.xyz, (float3)(gammaFactor));
}


// Exponential fog from Inigo Quilez
float3 applyFog(in float3  rgb,		// original color of the pixel
	in float distance,				// camera to point distance
	in float3  rayDir,				// camera to point vector
	in float3  lightDir)			// light direction
{
	float fogAmount = 1.0 - exp(-distance * 0.0001f);
	float lightIntensity = max(dot(rayDir, lightDir), 0.0);

	float3 fogColor = lerp(
		float3(0.5, 0.6, 0.8), // bluish
		float3(1.0, 0.9, 0.7), // yellowish
		pow(lightIntensity, 8.0)
	);

	return lerp(rgb, fogColor, fogAmount);
}


void mapNormals(in SamplerState Sampler, in Texture2D normalMap, in float2 texCoords, in float3 tangent, inout float3 normal)
{
	//sample normal from the map
	float3 texNormal = normalMap.Sample(Sampler, texCoords).xyz;
	texNormal = 2.0f * texNormal - 1.f;
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	float3 bitangent = cross(normal, tangent);
	float3x3 TBNMatrix = float3x3(tangent, bitangent, normal);
	normal = normalize(mul(texNormal.xyz, TBNMatrix));
}




//@TODO add blinn phong as well, and update this to the new light struct when it's ready

//will redo once I have a better light data definition current one is bulky and sucks
void applyPhong(
	in float3 alc, in float3 dlc, in float3 slc, in float specularPower,
	in float3 normal, in float3 wPos, in float3 lPos, in float3 eyePos,
	inout float4 colour)
{
	float3 invLightDir = normalize(lPos - wPos);

	float3 viewDir = (wPos - eyePos);

	float4 ambient = calcAmbient(alc, 1.);

	float dFactor = 0;
	float4 diffuse = calcDiffuse(invLightDir, normal, dlc, 1., dFactor);

	float4 specular = calcSpecularPhong(invLightDir, normal, slc, 1., viewDir, dFactor, specularPower);

	colour.xyz = (ambient.xyz + diffuse.xyz) * colour.xyz + specular.xyz;
}


//NVIDIA FORMULA FOR BLINN-PHONG:
//Ks * lightColour * facing * max(dot(N, H), 0)	
//Ks is material's specular colour
//lightColour is float4(slc, 1.0f) aka the colour of light
//facing is 1 or 0
//N is the normalized surface normal
//V is the normalized vector towards the viewpoint (pToEye)
//L is the normalized vector towars the light source (inverseLightDir)
//H is the normalized vector halfway between V and L (pToEye + inverseLightDir)


/*
//specular light blinn-phong
float3 halfVector = normalize(inverseLightDir + pToEye.xyz);
float sFactor  = pow( max( dot(input.normal.xyz, halfVector), 0.0f ), SpecularPower );
*/

//float3 reflectionVector = 2 * dot(input.normal, inverseLightDir.xyz) * input.normal - inverseLightDir.xyz;



// Cascaded shadow maps, can be optimized a fair bit though

// Cascaded shadow map settings
#define NUM_CASCADES		4
#define XYDIM				1024.f
#define XYDIM_INV			(1.f / XYDIM)
#define BIAS				0.0001f

//PCF settings
#define NUM_SAMPLES			4
#define STEP_SIZE			1.f												// Values for ns = 4 & ss = 1
#define SAMPLE_SPAN			(NUM_SAMPLES * STEP_SIZE)						// 4   * 1.f  = 4.f
#define EVEN_SAMPLE_OFFSET	((1 - (NUM_SAMPLES % 2)) * (STEP_SIZE * 0.5f))	// 1.f * 0.5f = 0.5f
#define SAMPLE_OFFSET		((SAMPLE_SPAN * .5f) - EVEN_SAMPLE_OFFSET)		// 2.f - .5f  = 1.5f 
#define PCF_WITH_SAMPLE 1	// PCF with gather is not tested
static const float4 perPixelRowDivisor = (float4) (1. / (float) (NUM_SAMPLES * NUM_SAMPLES));

// @TODO use gather rather than sample, should be faster...
float applyPCF(float4 shadowCoord, Texture2DArray<float> csms, SamplerState Sampler, int index)
{
	float4 shadowMapDists = (float4)(0.f);

	float percentageLit = 0.f;

#if PCF_WITH_SAMPLE
    for (float i = 0.f; i < NUM_SAMPLES; i += STEP_SIZE)
	{
        for (float j = 0.f; j < NUM_SAMPLES; j += STEP_SIZE)
		{
			float2 sampleCoord = float2(shadowCoord.x, -shadowCoord.y) + (float2(i, j) - SAMPLE_OFFSET) * XYDIM_INV;
            sampleCoord = sampleCoord / shadowCoord.w * .5f + 0.5f;
			
            shadowMapDists[j] = csms.Sample(Sampler, float3(sampleCoord.x, sampleCoord.y, (float)index));

			// This is the logical math version, but using float4 dot products is faster
			//percentageLit += step(shadowCoord.z, shadowMapDists[i+j] + 0.0001) / (NUM_SAMPLES * NUM_SAMPLES);
        }
 
        percentageLit += dot((shadowCoord.z < shadowMapDists + BIAS), perPixelRowDivisor);
    }
#else
	// Just force a 4x4 grid kernel
    for (float a = -1; a < 2.f; a += 2.f)
    {
        for (float b = -1; b < 2.f; b += 2.f)
        {
            float2 sampleCoord = float2(shadowCoord.x, -shadowCoord.y) + float2(a, b) * XYDIM_INV;
            sampleCoord = sampleCoord / shadowCoord.w * .5f + 0.5f;
			
            csms.Gather(Sampler, float3(sampleCoord.x, sampleCoord.y, (float) index));
        }
		
        percentageLit += dot((shadowCoord.z < shadowMapDists + BIAS), perPixelRowDivisor);
    }
#endif

	//percentageLit = smoothstep(0., 1., percentageLit);
	return percentageLit;
}



// Determine whether the pixel is shadowed and by how much (PCF)
float obscur(float depth, matrix lvpMatrix[NUM_CASCADES], float4 cascades, 
	float4 worldPos, Texture2DArray<float> csms, SamplerState Sampler)
{
	// Determine which cascade should be sampled
	float4 distComp = float4(depth > cascades[0], depth > cascades[1], depth > cascades[2], depth > cascades[3]);

	// Determine which cascade it is in, up to NUM_CASCADES (shadow range can be smaller than the draw distance)
	float fIndex = dot(float4(NUM_CASCADES > 0, NUM_CASCADES > 1, NUM_CASCADES > 2, NUM_CASCADES > 3), distComp);
	int index = (int)(min(fIndex, NUM_CASCADES - 1));

	// Using the selected cascade's light view projection matrix, determine the pixel's position in light space
	float4 shadowCoord = mul(worldPos, lvpMatrix[index]);

	// Compare the two - only the pixels closest to the light will be directly illuminated
	float lit = applyPCF(shadowCoord, csms, Sampler, index);
	return lit;
}