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


//will do once I have a better light data definition current one is bulky and sucks
float4 calcPhong(in float3 alc, in float3 dlc, in float3 slc)
{
	return float4(0., 0., 0., 0.);
}





float3 gammaCorrect(in float3 colour, in float3 gammaFactor)
{
	return pow(colour.xyz, gammaFactor);
}


float3 gammaCorrect(in float3 colour, in float gammaFactor)	//convenient overload nothing more
{
	return pow(colour.xyz, float3(gammaFactor, gammaFactor, gammaFactor));
}


//exponential fog from Inigo Quilez
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


//@TODO add blinn phong


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