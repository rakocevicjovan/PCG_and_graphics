struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

Texture2D shaderTexture;
Texture2D bloomTexture;
SamplerState Sampler;



float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	float4 HDRColour = shaderTexture.Sample(Sampler, input.tex);
	float4 bloomColour = bloomTexture.Sample(Sampler, input.tex);

	HDRColour += bloomColour;
	float4 result = float4(1., 1., 1., 1.) - exp(-HDRColour * 1.f);	//default exposure

	result.xyz = pow(result.xyz, float3(0.666666, 0.666666, 0.666666));
	result.w = 1.f;
	return result;
}