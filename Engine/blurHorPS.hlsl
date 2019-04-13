struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

Texture2D shaderTexture;
SamplerState SampleType;

static const float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	float2 tex_offset = float2(1.0f, 1.0f) / float2(1600.f, 900.f); // gets size of single texel
	float3 res = shaderTexture.Sample(SampleType, input.tex).rgb * weight[0]; // current fragment's contribution

	for (int i = 1; i < 5; ++i)
	{
		res += shaderTexture.Sample(SampleType, input.tex + float2(tex_offset.x * i, 0.0)).rgb * weight[i];
		res += shaderTexture.Sample(SampleType, input.tex - float2(tex_offset.x * i, 0.0)).rgb * weight[i];
	}

	return float4(res.x, res.y, res.z, 1.f);
}