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


struct PixelInputType{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 worldPos : WPOS;
};

Texture2D shaderTexture;
SamplerState SampleType;

//go 2-4 times higher on this when using blinn phong compared to phong
static const float SpecularPower = 8.f;




float3 applyFog(in float3  rgb,		// original color of the pixel
               in float distance,	// camera to point distance
               in float3  rayDir,   // camera to point vector
               in float3  sunDir )  // sun light direction
{
    float fogAmount = 1.0 - exp( -distance * 0.0001f );	//*b WHAT THE HELL IS "b" IN DENSITY MR QUILEZ???
    float moonIntensity = max( dot( rayDir, sunDir ), 0.0 );
    float3 fogColor  = lerp( float3(0.5,0.6,0.8), // bluish
                            float3(1.0,0.9,0.7), // yellowish
                            pow(moonIntensity,8.0) );
    return lerp( rgb, fogColor, fogAmount );
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



float4 LightPixelShader(PixelInputType input) : SV_TARGET{
	
	input.normal = normalize(input.normal);

	float3 lightDir = normalize(input.worldPos.xyz - lightPosition.xyz);
	float3 invLightDir = -lightDir;

	float3 viewDir = input.worldPos.xyz - eyePos.xyz;	
	float distance = length(viewDir);
	viewDir = viewDir / distance;
	float3 invViewDir = -viewDir;

	//texture colour
	float4 colour = shaderTexture.Sample(SampleType, input.tex);

	//calculate ambient light
	float4 ambient = calcAmbient(alc, ali);

	//calculate diffuse light
	float dFactor = 0.f;
	float4 diffuse = calcDiffuse(invLightDir, input.normal, dlc, dli, dFactor);
		
	//calculate specular light
	float4 specular = calcSpecular(invLightDir, input.normal, slc, sli, viewDir, dFactor);

	colour = (ambient + diffuse) * colour + specular;

	//apply fog
	//colour = float4(applyFog(colour.xyz, distance, viewDir, lightDir), 1.0f);

	//apply gamma correction
	colour.xyz = pow( colour.xyz, float3(1.0f/2.2f, 1.0f/2.2f, 1.0f/2.2f));

	colour.w = 1.f;
    return colour;
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








/*
float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat){
	const float m = mat.Shininess * 256.0f;
	float3 halfVec = normalize(toEye + lightVec);

	float roughnessFactor = (m + 8.0f)*pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
	float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);

	float3 specAlbedo = fresnelFactor * roughnessFactor;

	// Our spec formula goes outside [0,1] range, but we are
	// doing LDR rendering.  So scale it down a bit.
	specAlbedo = specAlbedo / (specAlbedo + 1.0f);

	return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}
*/