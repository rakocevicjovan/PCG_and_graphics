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
};

Texture2D shaderTexture : register(t0);
SamplerState Sampler;

static const float SpecularPower = 8.f;



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
	return saturate(float4(slc, 1.0f) * sFactor * sli * dFactor);
}



float4 main(PixelInputType input) : SV_TARGET{

	input.normal = normalize(input.normal);

	float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);
	float3 invLightDir = -lightDir;

	float3 viewDir = input.worldPos.xyz - eyePos.xyz;
	float distance = length(viewDir);
	viewDir = viewDir / distance;
	float3 invViewDir = -viewDir;




	float4 colour = shaderTexture.Sample(Sampler, input.tex);
	float4 ambient = calcAmbient(alc, ali);
	float dFactor = 0;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, dlc, dli, dFactor);

	float4 specular = calcSpecular(invLightDir, input.normal, slc, sli, viewDir, dFactor);

    colour.xyz = (ambient.xyz + diffuse.xyz) * colour.xyz + specular.xyz;

	colour.xyz = pow(colour.xyz, float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));

	return colour;
}