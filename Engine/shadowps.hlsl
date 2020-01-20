Texture2D depthMapTexture : register(t0);
Texture2D shaderTexture : register(t1);


SamplerState SampleTypeWrap  : register(s0);
SamplerState SampleTypeClamp : register(s1);


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
	float4 worldPosition : TEXTURE;
    float4 fragPosLightSpace : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};

static const float SpecularPower = 32.f;




float3 applyFog(in float3  rgb,		// original color of the pixel
	in float distance,	// camera to point distance
	in float3  rayDir,   // camera to point vector
	in float3  sunDir)  // sun light direction
{
	float fogAmount = 1.0 - exp(-distance * 0.0001f);	//*b WHAT THE HELL IS "b" IN DENSITY MR QUILEZ???
	float sunAmount = max(dot(rayDir, sunDir), 0.0);
	float3 fogColor = lerp(float3(0.5, 0.6, 0.8), // bluish
		float3(1.0, 0.9, 0.7), // yellowish
		pow(sunAmount, 8.0));
	return lerp(rgb, fogColor, fogAmount);
}

float3 darken(in float3 rgb) {
	return float3(lerp(rgb, float3(0.0f, 0.0f, 0.0f), 0.5f));
}


float4 calcAmbient(in float3 alc, in float ali) {
	return saturate(float4(alc, 1.0f) * ali);
}


float4 calcDiffuse(in float3 invLightDir, in float3 normal, in float3 dlc, in float dli, inout float dFactor) {

	dFactor = max(dot(normal, invLightDir), 0.0f);
	return saturate(float4(dlc, 1.0f) * dli * dFactor);
}


float4 calcSpecular(in float3 invLightDir, in float3 normal, in float3 slc, in float sli, in float3 invViewDir, in float dFactor) {

	//float3 halfVec = normalize(invLightDir + invViewDir);
	//float sFactor = pow(max(dot(normal, halfVec), 0.0f), SpecularPower);

	//float3 reflection = normalize( 2.f * normal - invLightDir);
	float3 reflection = normalize(reflect(invLightDir, normal));
	float sFactor = pow(saturate(dot(reflection, invViewDir)), SpecularPower);
	return saturate(float4(slc, 1.0f) * sFactor * sli * dFactor);	// sli
}





float4 main(PixelInputType input) : SV_TARGET
{
    float2 projectTexCoord;
    float closestDepth;
    float lightDepthValue;
    float lightIntensity;

	input.normal = normalize(input.normal);

	float4 texColour = shaderTexture.Sample(SampleTypeWrap, input.tex);
	float4 colour;

	//fragPosLightSpace is actually the screen position of pixels relative to the light source
	///and we just replicate the same procedure that the usual position would go through when being prepared for the fragment shader
    projectTexCoord.x =  input.fragPosLightSpace.x / input.fragPosLightSpace.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.fragPosLightSpace.y / input.fragPosLightSpace.w / 2.0f + 0.5f;

	float3 lightDir = normalize(input.worldPosition.xyz - lightPosition.xyz);
	float3 invLightDir = -lightDir;

	float3 viewDir = input.worldPosition.xyz - eyePos.xyz;
	float distance = length(viewDir);
	viewDir = viewDir / distance;
	float3 invViewDir = -viewDir;

	//calculate ambient light
	float4 ambient = calcAmbient(alc, ali);

	//calculate diffuse light
	float dFactor = 0;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, dlc, dli, dFactor);

	//calculate specular light
	float4 specular = calcSpecular(invLightDir, input.normal, slc, sli, viewDir, dFactor);

	colour = (ambient + diffuse) * texColour;

	//apply fog
	colour = float4(applyFog(colour.xyz, distance, viewDir, lightDir), 1.0f);

	//apply gamma correction
	colour.xyz = pow(abs(colour.xyz), float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
	colour.w = 1.0f;

	
    if((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y)){

        closestDepth = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;
        //lightDepthValue = ( (2.f * input.fragPosLightSpace.z - 500.01f ) / 499.99f ) / input.fragPosLightSpace.w;
		lightDepthValue = input.fragPosLightSpace.z / input.fragPosLightSpace.w;
        lightDepthValue -= 0.001f;

		if (lightDepthValue > closestDepth) {
			colour.xyz = darken(colour.xyz);
			colour.w = 1.0f;
			return colour;
		}
	}

	colour = colour + specular;
	colour.w = 1.0f;
	return colour;
}
