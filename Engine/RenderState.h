#pragma once

struct RenderState
{
	ID3D11VertexShader* vs;
	ID3D11PixelShader* ps;

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampler;

	std::vector<ID3D11ShaderResourceView*> texSRVs;

	D3D11_PRIMITIVE_TOPOLOGY topology;

	ID3D11Buffer* vBuffer;
	ID3D11Buffer* iBuffer;

	unsigned int stride;
	unsigned int offset;
};