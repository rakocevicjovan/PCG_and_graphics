struct PixelInputType {
	float4 position : SV_POSITION;
	float3 texCoord : TEXCOORD;
};

TextureCube cubeMapTexture;
SamplerState Sampler;


float4 CMFS(PixelInputType input) : SV_TARGET{

	float4 colour = cubeMapTexture.Sample(Sampler, input.texCoord);



	return colour;
}
