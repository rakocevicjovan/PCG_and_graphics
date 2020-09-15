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

#define NUM_CASCADES 4
#define XYDIM 1024.f
#define NUM_SAMPLES 2.f
#define SAMPLE_OFFSET NUM_SAMPLES * .5f

float applyPCF(float4 shadowCoord, float smWidth, Texture2DArray<float> csms, SamplerState Sampler, int index)
{
	float4 shadowMapDists = (float4)(0.f);
	float fIdx = (float)index;

	for (float i = 0; i < NUM_SAMPLES; i += 1.)
	{
		for (float j = 0; j < NUM_SAMPLES; j += 1.)
		{
			float2 smpCrd = (float2)(0.f);
			smpCrd.x = (shadowCoord.x  + (i - SAMPLE_OFFSET) / smWidth) / shadowCoord.w * .5f + 0.5f;
			smpCrd.y = (-shadowCoord.y + (j - SAMPLE_OFFSET) / smWidth) / shadowCoord.w * .5f + 0.5f;
			
			shadowMapDists[NUM_SAMPLES * i + j] += csms.Sample(Sampler, float3(smpCrd.x, smpCrd.y, fIdx)).x;

			/*
			shadowMapDists += csms.Load(int4(
											shadowCoord2.x * XYDIM. - 2 + i * 4,
											shadowCoord2.y * XYDIM. - 2 + j * 4,
											index,
											0)).x;
			*/

			// Works but a single dot product is faster
			//percentageLit += step(shadowCoord.z, shadowMapDists[i+j] + 0.0001) * .25f;
		}
	}

	float percentageLit = dot((shadowCoord.z < shadowMapDists + 0.00001f), (float4)(.25f));
	//percentageLit = smoothstep(0., 1., percentageLit);
	return percentageLit;
}



// Determine whether the pixel is shadowed and by how much (PCF)
float obscur(float depth, float smWidth, matrix lvpMatrix[NUM_CASCADES], float4 cascades, 
	float4 worldPos, Texture2DArray<float> csms, SamplerState Sampler)
{
	// Determine which cascade should be sampled
	float4 distComp = float4(depth > cascades[0], depth > cascades[1], depth > cascades[2], depth > cascades[3]);

	// Determine which cascade it is in, up to NUM_CASCADES (shadow range can be limited)
	float fIndex = dot(float4(NUM_CASCADES > 0, NUM_CASCADES > 1, NUM_CASCADES > 2, NUM_CASCADES > 3), distComp);
	int index = (int)(min(fIndex, NUM_CASCADES - 1));

	// Using the selected cascade's light view projection matrix, determine the pixel's position in light space
	float4 shadowCoord = mul(worldPos, lvpMatrix[index]);

	// Compare the two - only the pixels closest to the light will be directly illuminated
	// step: 1 if the x parameter is greater than or equal to the y parameter; otherwise, 0.
	float lit = applyPCF(shadowCoord, smWidth, csms, Sampler, index);
	return lit;
}