struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

float4 main(PixelInputType input) : SV_TARGET
{
	float4 colour = shaderTexture.Sample(SampleType, input.tex);
	return colour;
}