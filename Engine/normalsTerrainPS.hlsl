cbuffer LightBuffer
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

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);

SamplerState Sampler;

//go 2-4 times higher on this when using blinn phong compared to phong
static const float SpecularPower = 256.f;


float3 applyFog(in float3  rgb,		// original color of the pixel
	in float distance,	// camera to point distance
	in float3  rayDir,   // camera to point vector
	in float3  sunDir)  // sun light direction
{
	float fogAmount = 1.0 - exp(-distance * 0.0001f);	//*b WHAT THE HELL IS "b" IN DENSITY MR QUILEZ???
	float moonIntensity = max(dot(rayDir, sunDir), 0.0);
	float3 fogColor = lerp(float3(0.5, 0.6, 0.8), // bluish
		float3(1.0, 0.9, 0.7), // yellowish
		pow(moonIntensity, 8.0));
	return lerp(rgb, fogColor, fogAmount);
}



float4 calcAmbient(in float3 alc, in float ali)
{
	return saturate(float4(alc, 1.0f) * ali);
}


float4 calcDiffuse(in float3 invLightDir, in float3 normal, in float3 dlc, in float dli, inout float dFactor)
{
	dFactor = dFactor = clamp(0., 1., max(dot(normal, invLightDir), 0.0f));
	return saturate(float4(dlc, 1.0f) * dli * dFactor);
}


float4 calcSpecular(in float3 invLightDir, in float3 normal, in float3 slc, in float sli, in float3 invViewDir, in float dFactor)
{
	float3 reflection = normalize(reflect(invLightDir, normal));
	float sFactor = pow(saturate(dot(reflection, invViewDir)), SpecularPower);
	return saturate(float4(slc, 1.0f) * sFactor * sli * dFactor);	// sli
}


float map(float value, float min1, float max1, float min2, float max2)
{
	return min2 + ((value - min1) / (max1 - min1)) * (max2 - min2);
}



float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	float4 texNormal = tex1.Sample(Sampler, input.tex);	//sample normal from the map
	texNormal = 2.0f * texNormal - 1.f;					//remap it to [-1, 1]

	//removes projection of tangent onto normal from tangent so they are orthogonal for sure
	input.tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
	float3 bitangent = cross(input.normal, input.tangent);
	float3x3 TBNMatrix = float3x3(input.tangent, bitangent, input.normal);
	input.normal = normalize(mul(texNormal.xyz, TBNMatrix));
	float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);
	float3 invLightDir = -lightDir;

	float3 viewDir = input.worldPos.xyz - eyePos.xyz;
	float distance = length(viewDir);
	viewDir = viewDir / distance;
	float3 invViewDir = -viewDir;


	float4 colour = tex0.Sample(Sampler, input.tex);


	//calculate ambient light
	float4 ambient = calcAmbient(alc, ali);

	//calculate diffuse light
	float dFactor = 0.f;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, dlc, .1, dFactor);

	//calculate specular light
	float4 specular = calcSpecular(invLightDir, input.normal, slc, .3, viewDir, dFactor);

	colour = (ambient + diffuse) * colour + specular;

	//colour = float4(applyFog(colour.xyz, distance, viewDir, lightDir), 1.0f);	//apply fog

	colour.xyz = pow(colour.xyz, float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));	//apply gamma correction

	colour.w = 1.f;
	return colour;
}