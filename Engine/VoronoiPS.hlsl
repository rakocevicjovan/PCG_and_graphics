struct PixelInputType 
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

Texture2D shaderTexture;
SamplerState SampleType;

float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	//blabla...
	float4 colour = float4(0, 0, 0, 1);
	return colour;
}