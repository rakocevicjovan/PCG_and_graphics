#pragma once

// Initial idea, however it seems too big to be used the way I intend to.
struct RenderState
{
	ID3D11VertexShader* vs;
	ID3D11PixelShader* ps;

	ID3D11InputLayout* _layout;
	ID3D11SamplerState* _sampler;

	std::vector<ID3D11ShaderResourceView*> _texSRVs;

	D3D11_PRIMITIVE_TOPOLOGY _topology;

	ID3D11Buffer* _vBuffer;
	ID3D11Buffer* _iBuffer;

	unsigned int _stride;
	unsigned int _offset;
};