struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

Texture2D shaderTexture;
SamplerState SampleType;



float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	float4 colour = shaderTexture.Sample(SampleType, input.tex);

	float brightness = dot(colour.rgb, colour.rgb);//float3(0.2126, 0.7152, 0.0722));

	colour = smoothstep(.8, 1., brightness) * colour;

	return colour;
}