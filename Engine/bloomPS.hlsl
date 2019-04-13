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
	

	return colour;
}