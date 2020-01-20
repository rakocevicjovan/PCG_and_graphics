Texture2D projectionTexture : register(t0);
Texture2D shaderTexture : register(t1);

SamplerState SampleType;

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
    float4 viewPosition : TEXCOORD1;
};


float4 main(PixelInputType input) : SV_TARGET
{
    float4 color;
    float3 lightDir;
    float lightIntensity;
    float4 textureColor;
    float2 projectTexCoord;
    float4 projectionColor;

    color = alc * ali;
    lightDir = -lightPosition.xyz;
    lightIntensity = saturate(dot(input.normal, lightDir));

    if(lightIntensity > 0.0f)
	{
        // Determine the light color based on the diffuse color and the amount of light intensity.
        color += (dlc * dli * lightIntensity);
    }

    color = saturate(color);
    textureColor = shaderTexture.Sample(SampleType, input.tex);

    color = color * textureColor;

	// Now we calculate the projection coordinates for sampling the projected texture from.
	// These coordinates are the position the vertex is being viewed from by the location of the projection view point. 
	// The coordinates are translated into 2D screen coordinates and moved into the 0.0f to 1.0f range from the -0.5f to +0.5f range.

    // Calculate the projected texture coordinates.
    projectTexCoord.x =  input.viewPosition.x / input.viewPosition.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.viewPosition.y / input.viewPosition.w / 2.0f + 0.5f;

	// Next we need to check if the coordinates are in the 0.0f to 1.0f range.
	// If they are not in that range then this pixel is not in the projection area so it is just illuminated and textured as normal.
	// However if it is inside the 0.0f to 1.0f range then this pixel is inside the projected texture area and we need to apply the projected texture to the output pixel.

    // Determine if the projected coordinates are in the 0 to 1 range.  If it is then this pixel is inside the projected view port.
    if((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y)){

		// Sample the projection texture using the projected texture coordinates and then set the output pixel color to be the projected texture color.

        // Sample the color value from the projection texture using the sampler at the projected texture coordinate location.
        projectionColor = projectionTexture.Sample(SampleType, projectTexCoord);

        // Set the output color of this pixel to the projection texture overriding the regular color value.
        color = projectionColor;
    }

    return color;
}