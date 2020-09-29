#include "GeoClipmap.h"
#include "VertSignature.h"
#include "Shader.h"


GeoClipmap::GeoClipmap(UINT numLayers, UINT edgeSizeLog2)
	: _numLayers(numLayers), _w(pow(2, edgeSizeLog2) - 1)
{
	_x = (_w + 1) / 4;	// Size of the outer layer block in vertices
	_gapSize = _x * 3;	// Cardinal gaps
}



void GeoClipmap::init(ID3D11Device* device)
{
	// Vertex buffers
	std::vector<SVec2> vertXYs;
	vertXYs.reserve(_x * _x);

	// @TODO replace with bufferless, no need for it on newer GPUs
	for (int i = 0; i < _x; ++i)
	{
		for (int j = 0; j < _x; ++j)
			vertXYs.emplace_back(j, i);	// Apply some scaling here, not sure how to determine it.
	}

	VBuffer centerBuffer(device, nullptr, 0, sizeof(SVec2));

	VBuffer blockBuffer(device, vertXYs.data(), vertXYs.size() * sizeof(SVec2), sizeof(SVec2));

	// Vertex shader
	VertSignature vertSig;
	vertSig.addAttribute(VAttribSemantic::POS, VAttribType::FLOAT2, 1u, 4);
	std::vector<D3D11_INPUT_ELEMENT_DESC> inLayDesc = vertSig.createVertInLayElements();

	D3D11_BUFFER_DESC bufferDesc = CBuffer::createDesc(sizeof(GeoClipmapBuffer));

	VertexShader vs(device, L"GeoClipmapVS.hlsl", inLayDesc, { bufferDesc });


	// Textures - 2 per layer, but instead packed into two arrays of _numLayers for faster binding
	D3D11_TEXTURE2D_DESC hmDesc = Texture::create2DTexDesc(_w, _w, DXGI_FORMAT_R32_FLOAT,
		D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0u, 0u, 1u,
		_numLayers);

	_heightMap.create(device, &hmDesc, nullptr);

	D3D11_TEXTURE2D_DESC nmDesc = Texture::create2DTexDesc(2.f * _w, 2.f * _w, DXGI_FORMAT_R8G8B8A8_SNORM,
		D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0u, 0u, 1u,
		_numLayers);

	_normalMap.create(device, &nmDesc, nullptr);
	

}



void GeoClipmap::update(ID3D11DeviceContext* context)
{

}



void GeoClipmap::draw(ID3D11DeviceContext* context)
{

}