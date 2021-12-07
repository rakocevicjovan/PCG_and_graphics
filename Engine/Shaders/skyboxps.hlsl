struct PixelInputType
{
	float4 position : SV_POSITION;
	float3 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

TextureCube cubeMapTexture : register(t0);
SamplerState Sampler : register(s0);


float4 main(PixelInputType input) : SV_TARGET
{
    return cubeMapTexture.Sample(Sampler, input.texCoord);
}


/* @TODO move to a separate shader, its kinda cool, this was a test
float3 spPos = getSpherical(-input.normal);
float3 wat = gridSpherical(spPos, 20., 20.);
float3 cart = getCartesian(wat);
float2 uv = cart.xy;

float4 colour = (float4) 0.;

float2 gridCell = floor(uv);
float2 gridOffset = frac(uv) - .5;

for (int i = -1; i < 2; ++i)
{
	for (int j = -1; j < 2; ++j)
	{

		float2 adjCell = gridCell + float2(i, j);
		float2 adjCellPoint = getCellStar(adjCell);
		float2 adjGridOffset = gridOffset - float2(i, j);

		float2 rekt = adjGridOffset - adjCellPoint;
		float dToAdj = length(rekt);

		colour.xyz += (float3)smoothstep(.4, .1, dToAdj);
	}
}
*/