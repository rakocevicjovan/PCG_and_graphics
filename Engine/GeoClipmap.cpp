#include "GeoClipmap.h"
#include "VertSignature.h"
#include "Shader.h"



GeoClipmap::GeoClipmap(UINT numLayers, UINT edgeSizeLog2, float xzScale)
	: _numLayers(numLayers), _edgeVertCount(pow(2, edgeSizeLog2) - 1), _coreVertSpacing(xzScale)
{
	_blockEdgeVertCount = (_edgeVertCount + 1) / 4;	// Size of the outer layer block in vertices
	_gapSize = _blockEdgeVertCount * 3;	// Cardinal gaps
	_offsets.reserve(_numLayers);

	// Texture size
	_texSize.first = _edgeVertCount + 2 * _numLayers * _blockEdgeVertCount;
}



void GeoClipmap::init(ID3D11Device* device)
{
	// Vertex buffers
	std::vector<SVec2> vertXYs;
	vertXYs.reserve(_blockEdgeVertCount * _blockEdgeVertCount);

	// @TODO replace with bufferless, no need for it on newer GPUs
	for (int i = 0; i < _blockEdgeVertCount; ++i)
	{
		for (int j = 0; j < _blockEdgeVertCount; ++j)
			vertXYs.emplace_back(j, i);
	}

	VBuffer centerBuffer(device, nullptr, 0, sizeof(SVec2));

	VBuffer blockBuffer(device, vertXYs.data(), vertXYs.size() * sizeof(SVec2), sizeof(SVec2));

	// Vertex shader
	VertSignature vertSig;
	vertSig.addAttribute(VAttribSemantic::POS, VAttribType::FLOAT2, 1u, 4u);	// Will be backed into float16
	std::vector<D3D11_INPUT_ELEMENT_DESC> inLayDesc = vertSig.createVertInLayElements();

	D3D11_BUFFER_DESC bufferDesc = CBuffer::createDesc(sizeof(GeoClipmapBuffer));

	VertexShader vs(device, L"GeoClipmapVS.hlsl", inLayDesc, { bufferDesc });

	/// TEXTURE SIZE IS NOT EDGE VERT COUNT IT SHOULD BE WAY BIGGER!
	// Textures - 2 per layer, but instead packed into two arrays of _numLayers for faster binding
	D3D11_TEXTURE2D_DESC hmDesc = Texture::create2DTexDesc(_edgeVertCount, _edgeVertCount, DXGI_FORMAT_R32_FLOAT,
		D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0u, 0u, 1u,
		_numLayers);

	_heightMap.create(device, &hmDesc, nullptr);

	D3D11_TEXTURE2D_DESC nmDesc = Texture::create2DTexDesc(2.f * _edgeVertCount, 2.f * _edgeVertCount, DXGI_FORMAT_R8G8B8A8_SNORM,
		D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0u, 0u, 1u,
		_numLayers);

	_normalMap.create(device, &nmDesc, nullptr);

	// Offsets towards bottom left per layer, starting with central block's offset
	SVec2 acumulatedOffset(-0.5f * _edgeVertCount * _coreVertSpacing);
	_offsets.push_back(acumulatedOffset);

	// Vertex spacing doubles every iteration, including the first layer, offsets increase quadratically
	float baseBlockSize = _blockEdgeVertCount * _coreVertSpacing;

	for (int i = 1; i < _numLayers; ++i)
	{
		int scaleModifier = 1 << i;	// 2, 4, 8...
		SVec2 nthLayerOffset(scaleModifier * baseBlockSize);

		acumulatedOffset -= nthLayerOffset;	// Shift by the width of the new layer
		_offsets.push_back(acumulatedOffset);
	}
	
}



void GeoClipmap::update(ID3D11DeviceContext* context)
{

}



void GeoClipmap::draw(ID3D11DeviceContext* context)
{
}